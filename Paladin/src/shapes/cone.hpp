//
//  cone.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef cone_hpp
#define cone_hpp

#include "core/header.h"
#include "core/shape.hpp"

PALADIN_BEGIN

/**
 * 圆锥隐式方程
 * ((hx)/r)^2 + ((hx)/r)^2 - (z - h)^2 = 0
 *
 * 参数关系如下
 * φ = u φmax
 * x = r(1 - v)cosφ
 * y = r(1 - v)sinφ
 * z = v * h
 */
class Cone : public Shape {
public:

    Cone(const Transform *o2w, const Transform *w2o, bool reverseOrientation,
         Float height, Float radius, Float phiMax): Shape(o2w, w2o, reverseOrientation),
    _radius(radius),
    _height(height),
    _phiMax(degree2radian(clamp(phiMax, 0, 360))) {}
    
    virtual void init() {
        _invArea = 1 / area();
    }
    
    virtual AABB3f objectBound() const {
        Point3f p1 = Point3f(-_radius, -_radius, 0);
        Point3f p2 = Point3f(_radius, _radius, _height);
        return AABB3f(p1, p2);
    }

    virtual bool intersect(const Ray &ray, Float *tHit, SurfaceInteraction *isect, bool testAlphaTexture) const;

    virtual bool intersectP(const Ray &ray, bool testAlphaTexture) const;

    // 圆锥侧面积公式就不写推导过程了
    // 可以用回转体面积公式，也可以用小学生方法将圆锥展开成扇形，求扇形面积
    virtual Float area() const {
        return _radius * std::sqrt((_height * _height) + (_radius * _radius)) * _phiMax / 2;
    }

    /**
     * 均匀采样圆锥，可以将圆锥展开成扇形，均匀采样扇形
     */
    virtual Interaction samplePos(const Point2f &u, Float *pdf) const;
    
protected:

    const Float _radius, _height, _phiMax;
};

PALADIN_END

#endif /* cone_hpp */
