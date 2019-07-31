//
//  cylinder.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef cylinder_hpp
#define cylinder_hpp

#include "header.h"
#include "shape.hpp"

PALADIN_BEGIN


/*
圆柱参数方程如下
φ = u φmax
x = r cos φ
y = r sin φ
z = zmin + v(zmax − zmin).
*/
class Cylinder : public Shape {
public:
    
    Cylinder(const Transform *ObjectToWorld, const Transform *WorldToObject,
             bool reverseOrientation, Float radius, Float zMin, Float zMax,
             Float _phiMax)
    : Shape(ObjectToWorld, WorldToObject, reverseOrientation),
    _radius(radius),
    _zMin(std::min(zMin, zMax)),
    _zMax(std::max(zMin, zMax)),
    _phiMax(degree2radian(clamp(_phiMax, 0, 360))) {

    } 

    virtual AABB3f objectBound() const {
        return AABB3f(Point3f(-_radius, -_radius, _zMin),
                    Point3f(_radius, _radius, _zMax));
    }
    
    /*
     1.求解直线与圆柱的交点
     2.判断交点是否在ray的范围内
     3.判断交点是否在圆柱的有效范围内
    */
    virtual bool intersect(const Ray &ray, Float *tHit, SurfaceInteraction *isect, bool testAlphaTexture) const;

    virtual bool intersectP(const Ray &ray, bool testAlphaTexture) const;

    virtual Float area() const {
        return (_zMax - _zMin) * _radius * _phiMax;
    }

    virtual Interaction sampleA(const Point2f &u, Float *pdf) const;
    
protected:
    const Float _radius;
    const Float _zMin;
    const Float _zMax;
    const Float _phiMax;
};

PALADIN_END

#endif /* cylinder_hpp */
