//
//  fresnelblend.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/2/14.
//

#include "fresnelblend.hpp"

PALADIN_BEGIN

// FresnelBlend
Spectrum FresnelBlend::f(const Vector3f &wo, const Vector3f &wi) const {
    auto pow5 = [](Float v) { return (v * v) * (v * v) * v; };
    Spectrum diffuse = (28.f / (23.f * Pi)) * _Rd * (Spectrum(1.f) - _Rs) *
                        (1 - pow5(1 - .5f * Frame::absCosTheta(wi))) *
                        (1 - pow5(1 - .5f * Frame::absCosTheta(wo)));
    Vector3f wh = wi + wo;
    if (wh.isZero()) {
        return Spectrum(0);
    }
    wh = normalize(wh);
    Spectrum specular = _distribution->D(wh) /
        (4 * absDot(wi, wh) * std::max(Frame::absCosTheta(wi), Frame::absCosTheta(wo))) *
        schlickFresnel(dot(wo, wh));
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
    return .5f * (Frame::absCosTheta(wi) * InvPi + pdf_wh / (4 * dot(wo, wh)));
}

std::string FresnelBlend::toString() const {
    return std::string("[ FresnelBlend Rd: ") + _Rd.ToString() +
    std::string(" Rs: ") + _Rs.ToString() +
    std::string(" distribution: ") + _distribution->toString() +
    std::string(" ]");
}

PALADIN_END
