//
//  pointlight.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "pointlight.hpp"
#include "core/sampling.hpp"

PALADIN_BEGIN

Spectrum PointLight::sampleLi(const Interaction &ref, const Point2f &u,
                               Vector3f *wi, Float *pdf,
                               VisibilityTester *vis) const {
    *wi = normalize(_pLight - ref.p);
    // todo
    *pdf = 1.0f;
    *vis = VisibilityTester(ref, Interaction(_pLight, ref.time, mediumInterface));
    return _I / distanceSquared(_pLight, ref.p);
}

Float PointLight::pdfLi(const Interaction &, const Vector3f &) const {
    return 0;
}

void PointLight::pdfLe(const Ray &, const Normal3f &, Float *pdfPos,
                        Float *pdfDir) const {
    *pdfPos = 0;
    *pdfDir = uniformSpherePdf();
}

PALADIN_END