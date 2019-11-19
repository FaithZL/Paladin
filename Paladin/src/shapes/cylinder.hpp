//
//  cylinder.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef cylinder_hpp
#define cylinder_hpp

#include "core/header.h"
#include "core/shape.hpp"

PALADIN_BEGIN


/**
 * 圆柱参数方程如下
 * φ = u φmax
 * x = r cos φ
 * y = r sin φ
 * z = zmin + v(zmax − zmin).
 */
class Cylinder : public Shape {
public:
    
    Cylinder(shared_ptr<const Transform> ObjectToWorld, shared_ptr<const Transform> WorldToObject,
             bool reverseOrientation, Float radius, Float zMin, Float zMax,
             Float _phiMax)
    : Shape(ObjectToWorld, WorldToObject, reverseOrientation),
    _radius(radius),
    _zMin(std::min(zMin, zMax)),
    _zMax(std::max(zMin, zMax)),
    _phiMax(degree2radian(clamp(_phiMax, 0, 360))) {
        init();
    }
    
    virtual void init() override {
        
    }

    virtual AABB3f objectBound() const override {
        return AABB3f(Point3f(-_radius, -_radius, _zMin),
                    Point3f(_radius, _radius, _zMax));
    }
    
    virtual nloJson toJson() const override {
        return nloJson();
    }
    
    /**
     * 1.求解直线与圆柱的交点
     * 2.判断交点是否在ray的范围内
     * 3.判断交点是否在圆柱的有效范围内
     */
    virtual bool intersect(const Ray &ray, Float *tHit, SurfaceInteraction *isect, bool testAlphaTexture) const override;

    virtual bool intersectP(const Ray &ray, bool testAlphaTexture) const override;

    virtual Float area() const override {
        return (_zMax - _zMin) * _radius * _phiMax;
    }

    virtual Interaction samplePos(const Point2f &u, Float *pdf) const override;
    
protected:
    const Float _radius;
    const Float _zMin;
    const Float _zMax;
    const Float _phiMax;
};

CObject_ptr createCylinder(const nloJson &param, const Arguments &lst);

PALADIN_END

#endif /* cylinder_hpp */
