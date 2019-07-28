//
//  sphere.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "sphere.hpp"
#include "sampling.hpp"

PALADIN_BEGIN

void Sphere::init() {
    _invArea = 1.f / area();
}

/*
 可以把球体当做一个180°的圆弧以z轴旋转360°得到的一个回转体
 球体的一部分则可以当做一个圆弧以z轴旋转φ
 由高等数学知识可得，回转体的表面积为
 area = φ * ∫f(z) * sqrt(1 + (f'(z))^2)dz (from zMin to zMax)
 f(z) = sqrt(r^2 - z^2)
 f'(z) = -(z / sqrt(r^2 - z^2))
 积分可得 area = φ * r * (zMax - zMin)
 */
Float Sphere::area() const {
    return _phiMax * _radius * (_zMax - _zMin);
}

Bounds3f Sphere::objectBound() const {
    return Bounds3f(Point3f(-_radius, -_radius, _zMin),
                    Point3f(_radius, _radius, _zMax));
}

bool Sphere::intersect(const Ray &ray, Float *tHit, SurfaceInteraction *isect, bool testAlphaTexture) const {
    return false;
}

bool Sphere::intersectP(const Ray &ray, bool testAlphaTexture) const {
    return false;
}

Interaction Sphere::sample(const Point2f &u, Float *pdf) const {
    return Interaction();
}

Interaction Sphere::sample(const paladin::Interaction &ref, const Point2f &u, Float *pdf) const {
    return Interaction();
}

Float Sphere::pdfW(const paladin::Interaction &ref, const Vector3f &wi) const {
    Point3f pCenter = _objectToWorld->exec(Point3f(0,0,0));
    // Point3f origin = ref.pos; todo
    Point3f origin = offsetRayOrigin(ref.pos, ref.pError, ref.normal, wi);
    if (distanceSquared(pCenter, origin) <= _radius * _radius) {
        // 如果在球内
        return Shape::pdfW(ref, wi);
    }
    
    // 如果在球外，则进行圆锥采样
    Float sinThetaMax2 = _radius * _radius / distanceSquared(ref.pos, pCenter);
    Float cosThetaMax = std::sqrt(std::max((Float)0, 1 - sinThetaMax2));
    return uniformConePdf(cosThetaMax);
}

PALADIN_END
