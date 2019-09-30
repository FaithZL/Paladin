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
    
    shading.normal = normal;
    shading.dpdu = dpdu;
    shading.dpdv = dpdv;
    shading.dndu = dndu;
    shading.dndv = dndv;
    
    if (shape &&
        (shape->reverseOrientation ^ shape->transformSwapsHandedness)) {
        normal *= -1;
        shading.normal *= -1;
    }
}

void SurfaceInteraction::computeDifferentials(const RayDifferential &ray) const {
    if (ray.hasDifferentials) {
        // 平面方程为 ax + by + cz = d
        // 法向量为n(a,b,c),平面上的点p(x,y,z)
        // d = n · p
        Float d = dot(normal, pos);

    } else {
        dudx = dvdx = 0;
        dudy = dvdy = 0;
        dpdx = dpdy = Vector3f(0, 0, 0);
    }
}

void SurfaceInteraction::setShadingGeometry(const Vector3f &dpdus,
                                            const Vector3f &dpdvs,
                                            const Normal3f &dndus,
                                            const Normal3f &dndvs,
                                            bool orientationIsAuthoritative) {

    shading.normal = normalize((Normal3f)cross(dpdus, dpdvs));
    
    if (shape && (shape->reverseOrientation ^ shape->transformSwapsHandedness)) {
        shading.normal = -shading.normal;
    }

    if (orientationIsAuthoritative) {
        normal = faceforward(normal, shading.normal);
    } else {
        shading.normal = faceforward(shading.normal, normal);
    }

    shading.dpdu = dpdus;
    shading.dpdv = dpdvs;
    shading.dndu = dndus;
    shading.dndv = dndvs;
}

PALADIN_END
