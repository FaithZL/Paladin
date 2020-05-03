//
//  disk.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef disk_hpp
#define disk_hpp

#include "core/header.h"
#include "core/shape.hpp"

PALADIN_BEGIN

/**
 * 圆盘，可以是扇形(不完整的圆盘)，有内径跟外径
 */
class Disk : public Shape {
public:
    
    Disk(const shared_ptr<const Transform> &objectToWorld, const shared_ptr<const Transform> &worldToObject,
         bool reverseOrientation, Float height, Float radius, Float innerRadius,
         Float phiMax)
    : Shape(objectToWorld, worldToObject, reverseOrientation),
    _height(height),
    _radius(radius),
    _innerRadius(innerRadius),
    _phiMax(degree2radian(clamp(phiMax, 0, 360))) {
        init();
    }
    
    virtual void init() override {
        _invArea = 1 / area();
    }
    
    virtual nloJson toJson() const override {
        return nloJson();
    }
    
    virtual AABB3f objectBound() const override {
        return AABB3f(Point3f(-_radius, -_radius, _height),
                    Point3f(_radius, _radius, _height));
    }
    
    virtual bool intersect(const Ray &ray, Float *tHit, SurfaceInteraction *isect,
                   bool testAlphaTexture) const override;
    
    virtual bool intersectP(const Ray &ray, bool testAlphaTexture) const override;
   
    virtual Float area() const override {
        return _phiMax * 0.5 * (_radius * _radius - _innerRadius * _innerRadius);
    }
    
    virtual Interaction samplePos(const Point2f &u, Float *pdf) const override;
    
private:

    const Float _height;
    const Float _radius;
    const Float _innerRadius;
    const Float _phiMax;
};

CObject_ptr createDisk(const nloJson &param, const Arguments &lst);

PALADIN_END

#endif /* disk_hpp */
