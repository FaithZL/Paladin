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
 x = sinθcosφ
 y = sinθsinφ
 z = cosθ
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
    _zMax(clamp(zMax, -radius, radius)),
    _zMin(clamp(zMin, -radius, radius)),
    _phiMax(degree2radian(clamp(phiMax, 0, 360))),
    _minTheta(std::acos(clamp(std::min(_zMin, _zMax) / _radius, -1, 1))),
    _maxTheta(std::acos(clamp(std::max(_zMin, _zMax) / _radius, -1, 1))) {
        init();
    }
    
    void init();
    
    Bounds3f objectBound() const;
    
    bool intersect(const Ray &ray, Float *tHit, SurfaceInteraction *isect, bool testAlphaTexture) const;
    
    bool intersectP(const Ray &ray, bool testAlphaTexture) const;
    
    Float area() const;
    
    Interaction sample(const Point2f &u, Float *pdf) const;
    
    Interaction sample(const Interaction &ref, const Point2f &u, Float *pdf) const;
    
    Float pdfW(const Interaction &ref, const Vector3f &wi) const;
    
private:
    
    const Float _zMin;
    const Float _zMax;
    const Float _phiMax;
    const Float _radius;
    const Float _minTheta;
    const Float _maxTheta;
};

PALADIN_END

#endif /* sphere_hpp */
