//
//  reflection.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/2/14.
//

#include "reflection.hpp"


PALADIN_BEGIN

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
    Float D = _distribution->D(wh);
    Float G = _distribution->G(wo, wi);
    return _R * D * G * F / (4 * cosThetaI * cosThetaO);
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
    Vector3f tmpWo = normalize(wo);
    Vector3f wh = normalize(tmpWo + wi);
    Float d = dot(tmpWo, wh);
    return _distribution->pdfDir(tmpWo, wh) / (4 * d);
}

std::string MicrofacetReflection::toString() const {
    return std::string("[ MicrofacetReflection R: ") + _R.ToString() +
    std::string(" distribution: ") + _distribution->toString() +
    std::string(" fresnel: ") + _fresnel->toString() + std::string(" ]");
}


PALADIN_END
