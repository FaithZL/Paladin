//
//  sphere.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef sphere_hpp
#define sphere_hpp

#include "header.h"
#include "shape.hpp"

PALADIN_BEGIN
/*
 Sphere类，可以定义球体，以及部分球体
 可以把球体当做一个180°的圆弧以z轴旋转360°得到的一个回转体
 所有球体对象的object坐标系都是以球心为原点
 以z坐标的范围跟φ来裁剪球体，
 其中
 -r ≤ z ≤ r
 0 < φ ≤ 2π ，φ为转动的角度
 形状请自行脑补，完全ojbk

 球的参数方程可以表示为
 x = rsinθcosφ
 y = rsinθsinφ
 z = rcosθ
 
 x^2 + y^2 +z^2 − r^2 =0.

 表面参数uv为
 φ = u * φmax
 θ = θmin + v * (θmax - θmin)
 */
class Sphere : public Shape {
    
public:
    Sphere(Transform *objectToWorld,
           Transform *worldToObject,
           bool reverseOrientation,
           Float radius,
           Float zMax,
           Float zMin,
           Float phiMax):
    Shape(objectToWorld, worldToObject, reverseOrientation),
    _radius(radius),
    _zMax(clamp(std::max(zMax, zMin), -radius, radius)),
    _zMin(clamp(std::min(zMax, zMin), -radius, radius)),
    _phiMax(degree2radian(clamp(phiMax, 0, 360))),
    _thetaMin(std::acos(clamp(_zMin / _radius, -1, 1))),
    _thetaMax(std::acos(clamp(_zMax / _radius, -1, 1))) {
        init();
    }
    
    void init();
    
    Bounds3f objectBound() const;
    
    bool intersect(const Ray &ray, Float *tHit, SurfaceInteraction *isect, bool testAlphaTexture) const;
    
    bool intersectP(const Ray &ray, bool testAlphaTexture) const;
    
    Float area() const;
    
    Interaction sampleA(const Point2f &u, Float *pdf) const;
    
    Interaction sampleW(const Interaction &ref, const Point2f &u, Float *pdf) const;
    
    Float pdfW(const Interaction &ref, const Vector3f &wi) const;
    
private:
    
    const Float _zMin;
    const Float _zMax;
    const Float _phiMax;
    const Float _radius;
    const Float _thetaMin;
    const Float _thetaMax;
};

PALADIN_END

#endif /* sphere_hpp */
