//
//  bxdf.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "core/bxdf.hpp"
#include "core/sampling.hpp"

PALADIN_BEGIN

Float frDielectric(Float cosThetaI, Float etaI, Float etaT) {
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

Spectrum frConductor(Float cosThetaI, const Spectrum &etai,
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

Spectrum BxDF::sample_f(const Vector3f &wo,
                        Vector3f *wi,
                        const Point2f &sample,
                        Float *pdf,
                        BxDFType *sampledType) const {
    *wi = cosineSampleHemisphere(sample);
    if (wi->z < 0) {
        wi->z *= -1;
    }
    *pdf = pdfW(wo, *wi);
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

Float BxDF::pdfW(const Vector3f &wo, const Vector3f &wi) const {
    return sameHemisphere(wo, wi) ? absCosTheta(wi) * InvPi : 0;
}

PALADIN_END
