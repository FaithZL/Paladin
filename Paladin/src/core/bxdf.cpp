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


// BXDF 实现
Spectrum BxDF::sample_f(const Vector3f &wo,
                        Vector3f *wi,
                        const Point2f &sample,
                        Float *pdf,
                        BxDFType *sampledType) const {
    *wi = cosineSampleHemisphere(sample);
    if (wi->z < 0) {
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


// BSDF
BSDF::BSDF(const SurfaceInteraction &si, Float eta/* = 1*/)
: eta(eta),
_gNormal(si.normal),
_sNormal(si.shading.normal),
_sTangent(normalize(si.shading.dpdu)),
_tTangent(cross(_sNormal, _sTangent)) {

}

Spectrum BSDF::f(const Vector3f &woW, const Vector3f &wiW, BxDFType flags) const {
    Vector3f wi = worldToLocal(wiW);
    Vector3f wo = worldToLocal(woW);
    if (wo.z == 0) {
        return 0.;
    }
    bool reflect = dot(wiW, _gNormal) * dot(woW, _gNormal) > 0;
    Spectrum f(0.f);
    for (int i = 0; i < nBxDFs; ++i) {
        if (bxdfs[i]->matchesFlags(flags) &&
            ((reflect && (bxdfs[i]->type & BSDF_REFLECTION)) ||
             (!reflect && (bxdfs[i]->type & BSDF_TRANSMISSION)))) {
            f += bxdfs[i]->f(wo, wi);
        }
    }
    return f;
}

Spectrum BSDF::rho_hh(int nSamples, const Point2f *samples1,
                   const Point2f *samples2, BxDFType flags) const {
    Spectrum ret(0.f);
    for (int i = 0; i < nBxDFs; ++i) {
        if (bxdfs[i]->matchesFlags(flags)) {
            ret += bxdfs[i]->rho_hh(nSamples, samples1, samples2);
        }
    }
    return ret;
}

Spectrum BSDF::rho_hd(const Vector3f &wo, int nSamples, const Point2f *samples,
                   BxDFType flags) const {
    Spectrum ret(0.f);
    for (int i = 0; i < nBxDFs; ++i) {
        if (bxdfs[i]->matchesFlags(flags)) {
            ret += bxdfs[i]->rho_hd(wo, nSamples, samples);
        }
    }
    return ret;
}

Spectrum BSDF::sample_f(const Vector3f &woWorld, Vector3f *wiWorld,
                        const Point2f &u, Float *pdf, BxDFType type,
                        BxDFType *sampledType) const {
	// 计算符合type反射类型的组件数量
    int matchingComps = numComponents(type);
    if (matchingComps == 0) {
        *pdf = 0;
        if (sampledType) {
        	*sampledType = BxDFType(0);
        }
        return Spectrum(0);
    }
    // 根据随机变量随机选择bxdf组件
    // 目前的BSDF的各个bxdf组件是均匀分布，todo改为加权形式
    int comp = std::min((int)std::floor(u[0] * matchingComps), matchingComps - 1);
    
    BxDF *bxdf = nullptr;
    int count = comp;
    // 找到第comp个目标组件
    for (int i = 0; i < nBxDFs; ++i) {
        if (bxdfs[i]->matchesFlags(type) && count-- == 0) {
            bxdf = bxdfs[i];
            break;
        }
    }
    DCHECK(bxdf != nullptr);
    COUT << "BSDF::Sample_f chose comp = " << comp << " / matching = " <<
    matchingComps << ", bxdf: " << bxdf->toString();
    // 重新映射二维随机变量
    Point2f uRemapped(std::min(u[0] * matchingComps - comp, OneMinusEpsilon), u[1]);
    
    Vector3f wi, wo = worldToLocal(woWorld);
    if (wo.z == 0) {
    	return 0.;
    }
    *pdf = 0;
    if (sampledType) {
    	*sampledType = bxdf->type;
    }

    // 对选中的bxdf采样
    Spectrum f = bxdf->sample_f(wo, &wi, uRemapped, pdf, sampledType);

    COUT << "For wo = " << wo << ", sampled f = " << f << ", pdf = "
    << *pdf << ", ratio = " << ((*pdf > 0) ? (f / *pdf) : Spectrum(0.))
    << ", wi = " << wi;
    if (*pdf == 0) {
        if (sampledType) {
        	*sampledType = BxDFType(0);
        }
        return 0;
    }
    *wiWorld = localToWorld(wi);
    
    // 如果不包含高光反射，并且匹配的组件数大于1，才会累加pdf
    // 因为高光反射不能使用常规方法采样
    if (!(bxdf->type & BSDF_SPECULAR) && matchingComps > 1) {
        for (int i = 0; i < nBxDFs; ++i) {
            if (bxdfs[i] != bxdf && bxdfs[i]->matchesFlags(type)) {
                *pdf += bxdfs[i]->pdfDir(wo, wi);
            }
        }
    }

    if (matchingComps > 1) {
    	*pdf /= matchingComps;
    }
    
    if (!(bxdf->type & BSDF_SPECULAR)) {
        bool reflect = dot(*wiWorld, _gNormal) * dot(woWorld, _gNormal) > 0;
        f = 0.;
        for (int i = 0; i < nBxDFs; ++i) {
            if (bxdfs[i]->matchesFlags(type) &&
                ((reflect && (bxdfs[i]->type & BSDF_REFLECTION)) ||
                (!reflect && (bxdfs[i]->type & BSDF_TRANSMISSION)))) {
                f += bxdfs[i]->f(wo, wi);
            }
        }
    }
    COUT << "Overall f = " << f << ", pdf = " << *pdf << ", ratio = "
    << ((*pdf > 0) ? (f / *pdf) : Spectrum(0.));
    return f;
}

Float BSDF::pdfDir(const Vector3f &woWorld, const Vector3f &wiWorld,
                BxDFType flags) const {

    if (nBxDFs == 0.f) {
    	return 0.f;
    }
    Vector3f wo = worldToLocal(woWorld);
    Vector3f wi = worldToLocal(wiWorld);
    if (wo.z == 0) return 0.;
    Float pdf = 0.f;
    int matchingComps = 0;
    for (int i = 0; i < nBxDFs; ++i) {
        if (bxdfs[i]->matchesFlags(flags)) {
            ++matchingComps;
            pdf += bxdfs[i]->pdfDir(wo, wi);
        }
    }
    Float v = matchingComps > 0 ? pdf / matchingComps : 0.f;
    return v;
}

std::string BSDF::toString() const {
    std::string s = StringPrintf("[ BSDF eta: %f nBxDFs: %d", eta, nBxDFs);
    for (int i = 0; i < nBxDFs; ++i)
        s += StringPrintf("\n  bxdfs[%d]: ", i) + bxdfs[i]->toString();
    return s + std::string(" ]");
}

//  SpecularReflection
Spectrum SpecularReflection::sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample,
                  Float *pdf, BxDFType *sampledType) const {
    *wi = Vector3f(-wo.x, -wo.y, wo.z);
    *pdf = 1;
    return _fresnel->evaluate(cosTheta(*wi)) * _R / absCosTheta(*wi);
}

std::string SpecularReflection::toString() const {
    return std::string("[ SpecularReflection R: ") + _R.ToString() +
    std::string(" fresnel: ") + _fresnel->toString() + std::string(" ]");
}

// SpecularTransmission
Spectrum SpecularTransmission::sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample,
                  Float *pdf, BxDFType *sampledType) const {
    // 首先确定光线是进入或离开折射介质
    // 对象的法线都是向外的
    // 如果wo.z > 0，则说明，ray trace工作流的光线从物体外部射入物体
    bool entering = cosTheta(wo) > 0;
    Float etaI = entering ? _etaA : _etaB;
    Float etaT = entering ? _etaB : _etaA;
    // todo，这里代码可以优化一下
    if (!refract(wo, faceforward(Normal3f(0, 0, 1), wo), etaI / etaT, wi)) {
        return 0;
    }
    
    *pdf = 1;
    Spectrum ft = _T * (Spectrum(1.) - _fresnel.evaluate(cosTheta(*wi)));
    
    // 用于处理双向方法的情况，只有从光源射出的光线才需要乘以这个缩放因子
    if (_mode == TransportMode::Radiance) {
        ft *= (etaI * etaI) / (etaT * etaT);
    }
    return ft / absCosTheta(*wi);
}

std::string SpecularTransmission::toString() const {
    return std::string("[ SpecularTransmission: T: ") + _T.ToString() +
    StringPrintf(" etaA: %f etaB: %f ", _etaA, _etaB) +
    std::string(" fresnel: ") + _fresnel.toString() +
    std::string(" mode : ") +
    (_mode == TransportMode::Radiance ? std::string("RADIANCE")
     : std::string("IMPORTANCE")) +
    std::string(" ]");
}

// FresnelSpecular
Spectrum FresnelSpecular::sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                  Float *pdf, BxDFType *sampledType) const {
    Float F = frDielectric(cosTheta(wo), _etaA, _etaB);
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
Spectrum LambertianTransmission::sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                          Float *pdf, BxDFType *sampledType) const {
    *wi = cosineSampleHemisphere(u);
    if (wo.z > 0) {
        wi->z *= -1;
    }
    *pdf = pdfDir(wo, *wi);
    return f(wo, *wi);
}

Float LambertianTransmission::pdfDir(const Vector3f &wo, const Vector3f &wi) const {
    return sameHemisphere(wo, wi) ? 0 : absCosTheta(wi) * InvPi;
}

std::string LambertianTransmission::toString() const {
    return std::string("[ LambertianTransmission T: ") + _T.ToString() +
    std::string(" ]");
}

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


// MicrofacetReflection
Spectrum MicrofacetReflection::f(const Vector3f &wo, const Vector3f &wi) const {
    Float cosThetaO = absCosTheta(wo), cosThetaI = absCosTheta(wi);
    if (cosThetaI == 0 || cosThetaO == 0) {
        return Spectrum(0.);
    }
    Vector3f wh = wi + wo;
    if (wh.x == 0 && wh.y == 0 && wh.z == 0) {
        return Spectrum(0.);
    }
    wh = normalize(wh);
    Spectrum F = _fresnel->evaluate(dot(wi, wh));
    return _R * _distribution->D(wh) * _distribution->G(wo, wi) * F /
    (4 * cosThetaI * cosThetaO);
}

Spectrum MicrofacetReflection::sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                  Float *pdf, BxDFType *sampledType) const {
    if (wo.z == 0) {
        return 0.;
    }
    Vector3f wh = _distribution->sample_wh(wo, u);
    *wi = reflect(wo, wh);
    if (!sameHemisphere(wo, *wi)) {
        return Spectrum(0.f);
    }
    
    *pdf = _distribution->pdfDir(wo, wh) / (4 * dot(wo, wh));
    return f(wo, *wi);
}

Float MicrofacetReflection::pdfDir(const Vector3f &wo, const Vector3f &wi) const {
    if (!sameHemisphere(wo, wi)) {
        return 0;
    }
    Vector3f wh = normalize(wo + wi);
    return _distribution->pdfDir(wo, wh) / (4 * dot(wo, wh));
}

std::string MicrofacetReflection::toString() const {
    return std::string("[ MicrofacetReflection R: ") + _R.ToString() +
    std::string(" distribution: ") + _distribution->toString() +
    std::string(" fresnel: ") + _fresnel->toString() + std::string(" ]");
}

// MicrofacetTransmission
Spectrum MicrofacetTransmission::f(const Vector3f &wo, const Vector3f &wi) const {
    if (sameHemisphere(wo, wi)) {
        return 0;
    }
    
    Float cosThetaO = cosTheta(wo);
    Float cosThetaI = cosTheta(wi);
    if (cosThetaI == 0 || cosThetaO == 0) {
        return Spectrum(0);
    }
    // 确定入射光是从介质进入物体还是从物体离开进入介质
    Float eta = cosTheta(wo) > 0 ? (_etaB / _etaA) : (_etaA / _etaB);
    Vector3f wh = normalize(wo + wi * eta);
    if (wh.z < 0) {
        wh = -wh;
    }
    
    Spectrum F = _fresnel.evaluate(dot(wo, wh));
    
    Float sqrtDenom = dot(wo, wh) + eta * dot(wi, wh);
    // 双向方法用于区分传输方向
    Float factor = (_mode == TransportMode::Radiance) ? (1 / eta) : 1;
    
    return (Spectrum(1.f) - F) * _T *
    std::abs(_distribution->D(wh) * _distribution->G(wo, wi) * eta * eta *
             absDot(wi, wh) * absDot(wo, wh) * factor * factor /
             (cosThetaI * cosThetaO * sqrtDenom * sqrtDenom));
}

Spectrum MicrofacetTransmission::sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                  Float *pdf, BxDFType *sampledType) const {
    if (wo.z == 0) {
        return 0.;
    }
    Vector3f wh = _distribution->sample_wh(wo, u);
    // 确定入射光是从介质进入物体还是从物体离开进入介质
    Float eta = cosTheta(wo) > 0 ? (_etaA / _etaB) : (_etaB / _etaA);
    if (!refract(wo, (Normal3f)wh, eta, wi)) {
        return 0;
    }
    *pdf = pdfDir(wo, *wi);
    return f(wo, *wi);
}

Float MicrofacetTransmission::pdfDir(const Vector3f &wo, const Vector3f &wi) const {
    if (sameHemisphere(wo, wi)) {
        return 0;
    }
    Float eta = cosTheta(wo) > 0 ? (_etaB / _etaA) : (_etaA / _etaB);
    Vector3f wh = normalize(wo + wi * eta);
    
    Float sqrtDenom = dot(wo, wh) + eta * dot(wi, wh);
    Float dwh_dwi = std::abs((eta * eta * dot(wi, wh)) / (sqrtDenom * sqrtDenom));
    return _distribution->pdfDir(wo, wh) * dwh_dwi;
}

std::string MicrofacetTransmission::toString() const {
    return std::string("[ MicrofacetTransmission T: ") + _T.ToString() +
    std::string(" distribution: ") + _distribution->toString() +
    StringPrintf(" etaA: %f etaB: %f", _etaA, _etaB) +
    std::string(" fresnel: ") + _fresnel.toString() +
    std::string(" mode : ") +
    (_mode == TransportMode::Radiance ? std::string("RADIANCE")
     : std::string("IMPORTANCE")) +
    std::string(" ]");
}

// FresnelBlend
Spectrum FresnelBlend::f(const Vector3f &wo, const Vector3f &wi) const {
    auto pow5 = [](Float v) { return (v * v) * (v * v) * v; };
    Spectrum diffuse = (28.f / (23.f * Pi)) * _Rd * (Spectrum(1.f) - _Rs) *
					    (1 - pow5(1 - .5f * absCosTheta(wi))) *
					    (1 - pow5(1 - .5f * absCosTheta(wo)));
    Vector3f wh = wi + wo;
    if (wh.x == 0 && wh.y == 0 && wh.z == 0) {
    	return Spectrum(0);
    }
    wh = normalize(wh);
    Spectrum specular =
    	_distribution->D(wh) /
    	(4 * absDot(wi, wh) * std::max(absCosTheta(wi), absCosTheta(wo))) *
    	schlickFresnel(dot(wi, wh));
    return diffuse + specular;
}

Spectrum FresnelBlend::schlickFresnel(Float _cosTheta) const {
    auto pow5 = [](Float v) { return (v * v) * (v * v) * v; };
    return _Rs + pow5(1 - _cosTheta) * (Spectrum(1.) - _Rs);
}

Spectrum FresnelBlend::sample_f(const Vector3f &wo, Vector3f *wi,
                  const Point2f &uOrig, Float *pdf,
                  BxDFType *sampledType) const {
    Point2f u = uOrig;
    if (u[0] < .5) {
        u[0] = std::min(2 * u[0], OneMinusEpsilon);
        *wi = cosineSampleHemisphere(u);
        if (wo.z < 0) {
            wi->z *= -1;
        }
    } else {
        u[0] = std::min(2 * (u[0] - .5f), OneMinusEpsilon);
        Vector3f wh = _distribution->sample_wh(wo, u);
        *wi = reflect(wo, wh);
        if (!sameHemisphere(wo, *wi)) {
            return Spectrum(0.f);
        }
    }
    *pdf = pdfDir(wo, *wi);
    return f(wo, *wi);
}

Float FresnelBlend::pdfDir(const Vector3f &wo, const Vector3f &wi) const {
    if (!sameHemisphere(wo, wi)) {
    	return 0;
    }
    Vector3f wh = normalize(wo + wi);
    Float pdf_wh = _distribution->pdfDir(wo, wh);
    // 漫反射的pdf与菲涅尔反射的pdf取平均值
    return .5f * (absCosTheta(wi) * InvPi + pdf_wh / (4 * dot(wo, wh)));
}

std::string FresnelBlend::toString() const {
    return std::string("[ FresnelBlend Rd: ") + _Rd.ToString() +
    std::string(" Rs: ") + _Rs.ToString() +
    std::string(" distribution: ") + _distribution->toString() +
    std::string(" ]");
}

PALADIN_END
