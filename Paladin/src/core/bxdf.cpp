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
        Float pdfo = UniformHemispherePdf();
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
