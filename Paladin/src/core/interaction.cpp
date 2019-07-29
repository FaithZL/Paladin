//
//  interaction.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/7/11.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "interaction.hpp"
#include "shape.hpp"

PALADIN_BEGIN

SurfaceInteraction::SurfaceInteraction(
                                       const Point3f &p, const Vector3f &pError, const Point2f &uv,
                                       const Vector3f &wo, const Vector3f &dpdu, const Vector3f &dpdv,
                                       const Normal3f &dndu, const Normal3f &dndv, Float time, const Shape *shape,
                                       int faceIndex)
: Interaction(p, Normal3f(normalize(cross(dpdu, dpdv))), pError, wo, time, nullptr),
uv(uv),
dpdu(dpdu),
dpdv(dpdv),
dndu(dndu),
dndv(dndv),
shape(shape),
faceIndex(faceIndex) {
    // Initialize shading geometry from true geometry
    shading.normal = normal;
    shading.dpdu = dpdu;
    shading.dpdv = dpdv;
    shading.dndu = dndu;
    shading.dndv = dndv;
    
    // Adjust normal based on orientation and handedness
    if (shape &&
        (shape->reverseOrientation ^ shape->transformSwapsHandedness)) {
        normal *= -1;
        shading.normal *= -1;
    }
}

PALADIN_END
