//
//  bxdf.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "core/bxdf.hpp"
#include "math/sampling.hpp"
#include "core/interaction.hpp"
#include "math/interpolation.hpp"

PALADIN_BEGIN

Float FrDielectric(Float cosThetaI, Float etaI, Float etaT) {
    cosThetaI = clamp(cosThetaI, -1, 1);

    bool entering = cosThetaI > 0.f;
    // 如果如果入射角大于90° 
    // 则法线方向反了，cosThetaI取绝对值，对换两个折射率
    if (!entering) {
        std::swap(etaI, etaT);
        cosThetaI = std::abs(cosThetaI);
    }
    
    // 用斯涅耳定律计算sinThetaI
    Float sinThetaI = std::sqrt(std::max((Float)0, 1 - cosThetaI * cosThetaI));
    Float sinThetaT = etaI / etaT * sinThetaI;
    
    // 全内部反射情况
    if (sinThetaT >= 1) {
        return 1;
    }
    // 套公式
    Float cosThetaT = std::sqrt(std::max((Float)0, 1 - sinThetaT * sinThetaT));
    Float Rparl = ((etaT * cosThetaI) - (etaI * cosThetaT))
                / ((etaT * cosThetaI) + (etaI * cosThetaT));
    Float Rperp = ((etaI * cosThetaI) - (etaT * cosThetaT))
                / ((etaI * cosThetaI) + (etaT * cosThetaT));
    return (Rparl * Rparl + Rperp * Rperp) / 2;
}

Spectrum FrConductor(Float cosThetaI, const Spectrum &etai,
                     const Spectrum &etat, const Spectrum &kt) {
    cosThetaI = clamp(cosThetaI, -1, 1);
    Spectrum eta = etat / etai;
    Spectrum etak = kt / etai;
    
    Float cosThetaI2 = cosThetaI * cosThetaI;
    Float sinThetaI2 = 1. - cosThetaI2;
    Spectrum eta2 = eta * eta;
    Spectrum etak2 = etak * etak;
    
    Spectrum t0 = eta2 - etak2 - sinThetaI2;
    Spectrum a2plusb2 = Sqrt(t0 * t0 + 4 * eta2 * etak2);
    Spectrum t1 = a2plusb2 + cosThetaI2;
    Spectrum a = Sqrt(0.5f * (a2plusb2 + t0));
    Spectrum t2 = (Float)2 * cosThetaI * a;
    Spectrum Rs = (t1 - t2) / (t1 + t2);
    
    Spectrum t3 = cosThetaI2 * a2plusb2 + sinThetaI2 * sinThetaI2;
    Spectrum t4 = t2 * sinThetaI2;
    Spectrum Rp = Rs * (t3 - t4) / (t3 + t4);
    
    return 0.5 * (Rp + Rs);
}


// BXDF 实现
Spectrum BxDF::sample_f(const Vector3f &wo,
                        Vector3f *wi,
                        const Point2f &sample,
                        Float *pdf,
                        BxDFType *sampledType) const {
    *wi = cosineSampleHemisphere(sample);
    if (wo.z < 0) {
        wi->z *= -1;
    }
    *pdf = pdfDir(wo, *wi);
    return f(wo, *wi);
}

Spectrum BxDF::rho_hd(const Vector3f &wo, int nSamples, const Point2f *samples) const {
    Spectrum ret(0.0f);
    // ρhd(wo) = ∫[hemisphere]f(p,wi,wo)|cosθi|dwi
    // 蒙特卡洛方法采样估计积分值
    for (int i = 0; i < nSamples; ++i) {
        Vector3f wi;
        Float pdf = 0;
        Spectrum f = sample_f(wo, &wi, samples[i], &pdf);
        if (pdf > 0) {
            ret += f * absCosTheta(wi) / pdf;
        }
    }
    return ret / nSamples;
}

Spectrum BxDF::rho_hh(int nSamples, const Point2f *samplesWo, const Point2f *samplesWi) const {
    Spectrum ret(0.0f);
    // ρhh(wo) = (1/π)∫[hemisphere]∫[hemisphere]f(p,wi,wo)|cosθo * cosθi|dwidwo
    // 蒙特卡洛方法采样估计积分值
    for (int i = 0; i < nSamples; ++i) {
        Vector3f wo, wi;
        wo = uniformSampleHemisphere(samplesWo[i]);
        Float pdfo = uniformHemispherePdf();
        Float pdfi = 0;
        Spectrum r = sample_f(wo, &wi, samplesWi[i], &pdfi);
        if (pdfi > 0) {
            ret += r * absCosTheta(wi) * absCosTheta(wo) / (pdfi * pdfo);
        }
    }
    return ret / (Pi * nSamples);
}

Float BxDF::pdfDir(const Vector3f &wo, const Vector3f &wi) const {
    return sameHemisphere(wo, wi) ? absCosTheta(wi) * InvPi : 0;
}

// FresnelSpecular
Spectrum FresnelSpecular::sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                  Float *pdf, BxDFType *sampledType) const {
    Float F = FrDielectric(cosTheta(wo), _etaA, _etaB);
    if (u[0] < F) {
        *wi = Vector3f(-wo.x, -wo.y, wo.z);
        if (sampledType)
            *sampledType = BxDFType(BSDF_SPECULAR | BSDF_REFLECTION);
        *pdf = F;
        return F * _R / absCosTheta(*wi);
    } else {
        bool entering = cosTheta(wo) > 0;
        Float etaI = entering ? _etaA : _etaB;
        Float etaT = entering ? _etaB : _etaA;
        if (_thin) {
            etaI = etaT = 1.f;
        }
        
        if (!refract(wo, faceforward(Normal3f(0, 0, 1), wo), etaI / etaT, wi))
            return 0;
        Spectrum ft = _T * (1 - F);
        
        if (_mode == TransportMode::Radiance) {
            ft *= (etaI * etaI) / (etaT * etaT);
        }
        if (sampledType) {
            *sampledType = BxDFType(BSDF_SPECULAR | BSDF_TRANSMISSION);
        }
        *pdf = 1 - F;
        return ft / absCosTheta(*wi);
    }
}

std::string FresnelSpecular::toString() const {
    return std::string("[ FresnelSpecular R: ") + _R.ToString() +
    std::string(" T: ") + _T.ToString() +
    StringPrintf(" etaA: %f etaB: %f ", _etaA, _etaB) +
    std::string(" mode : ") +
    (_mode == TransportMode::Radiance ? std::string("RADIANCE")
     : std::string("IMPORTANCE")) +
    std::string(" ]");
}

// LambertianTransmission
//Spectrum LambertianTransmission::sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
//                          Float *pdf, BxDFType *sampledType) const {
//    *wi = cosineSampleHemisphere(u);
//    if (wo.z > 0) {
//        wi->z *= -1;
//    }
//    *pdf = pdfDir(wo, *wi);
//    return f(wo, *wi);
//}
//
//Float LambertianTransmission::pdfDir(const Vector3f &wo, const Vector3f &wi) const {
//    return sameHemisphere(wo, wi) ? 0 : absCosTheta(wi) * InvPi;
//}
//
//std::string LambertianTransmission::toString() const {
//    return std::string("[ LambertianTransmission T: ") + _T.ToString() +
//    std::string(" ]");
//}

// OrenNayar
Spectrum OrenNayar::f(const Vector3f &wo, const Vector3f &wi) const {
    Float sinThetaI = sinTheta(wi);
    Float sinThetaO = sinTheta(wo);
    // 计算max(0,cos(φi-φo))项
    // 由于三角函数耗时比较高，这里可以用三角恒等变换展开
    // cos(φi-φo) = cosφi * cosφo + sinφi * sinφo
    Float maxCos = 0;
    if (sinThetaI > 1e-4 && sinThetaO > 1e-4) {
        Float sinPhiI = sinPhi(wi), cosPhiI = cosPhi(wi);
        Float sinPhiO = sinPhi(wo), cosPhiO = cosPhi(wo);
        Float dCos = cosPhiI * cosPhiO + sinPhiI * sinPhiO;
        maxCos = std::max((Float)0, dCos);
    }
    
    Float sinAlpha, tanBeta;
    if (absCosTheta(wi) > absCosTheta(wo)) {
        sinAlpha = sinThetaO;
        tanBeta = sinThetaI / absCosTheta(wi);
    } else {
        sinAlpha = sinThetaI;
        tanBeta = sinThetaO / absCosTheta(wo);
    }
    return _R * InvPi * (_A + _B * maxCos * sinAlpha * tanBeta);
}

std::string OrenNayar::toString() const {
    return std::string("[ OrenNayar R: ") + _R.ToString() +
    StringPrintf(" A: %f B: %f ]", _A, _B);
}


PALADIN_END
