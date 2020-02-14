//
//  transmission.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/2/14.
//

#include "transmission.hpp"


PALADIN_BEGIN

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

PALADIN_END
