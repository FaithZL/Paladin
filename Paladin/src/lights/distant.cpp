//
//  distant.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/18.
//

#include "distant.hpp"

PALADIN_BEGIN

DistantLight::DistantLight(const Transform &LightToWorld, const Spectrum &L,
                           const Vector3f &wLight)
: Light((int)LightFlags::DeltaDirection, LightToWorld, MediumInterface()),
_L(L),
_wLight(normalize(LightToWorld.exec(wLight))) {}

Spectrum DistantLight::sampleLi(const Interaction &ref, const Point2f &u,
                                 Vector3f *wi, Float *pdf,
                                 VisibilityTester *vis) const {
    *wi = _wLight;
    *pdf = 1;
    Point3f pOutside = ref.pos + _wLight * (2 * _worldRadius);
    *vis =
    VisibilityTester(ref, Interaction(pOutside, ref.time, mediumInterface));
    return _L;
}

Spectrum DistantLight::power() const {
    return _L * Pi * _worldRadius * _worldRadius;
}

Float DistantLight::pdfLi(const Interaction &, const Vector3f &) const {
    return 0.f;
}

PALADIN_END
