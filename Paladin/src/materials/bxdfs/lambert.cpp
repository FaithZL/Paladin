//
//  lambert.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/2/12.
//

#include "lambert.hpp"

PALADIN_BEGIN

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


PALADIN_END
