//
//  disk.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef disk_hpp
#define disk_hpp

#include "header.h"
#include "shape.hpp"

PALADIN_BEGIN

/*
 圆盘，可以是扇形(不完整的圆盘)，有内径跟外径
 */
class Disk : public Shape {
public:
    
    Disk(const Transform *objectToWorld, const Transform *worldToObject,
         bool reverseOrientation, Float height, Float radius, Float innerRadius,
         Float phiMax)
    : Shape(objectToWorld, worldToObject, reverseOrientation),
    _height(height),
    _radius(radius),
    _innerRadius(innerRadius),
    _phiMax(degree2radian(clamp(phiMax, 0, 360))) {
        
    }
    
    void init() {
        _invArea = 1 / area();
    }
    
    Bounds3f objectBound() const;
    
    bool intersect(const Ray &ray, Float *tHit, SurfaceInteraction *isect,
                   bool testAlphaTexture) const;
    
    bool intersectP(const Ray &ray, bool testAlphaTexture) const;
   
    Float area() const {
        return _phiMax * 0.5 * (_radius * _radius - _innerRadius * _innerRadius);
    }
    
    Interaction sampleA(const Point2f &u, Float *pdf) const;
    
private:
    // 暂时搞不懂为何要这个参数，略显多余
    const Float _height;
    const Float _radius;
    const Float _innerRadius;
    const Float _phiMax;
};

PALADIN_END

#endif /* disk_hpp */
