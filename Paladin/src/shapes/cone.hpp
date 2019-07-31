//
//  cone.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef cone_hpp
#define cone_hpp

#include "header.h"
#include "shape.hpp"

PALADIN_BEGIN

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
    
    virtual Bounds3f objectBound() const {
        Point3f p1 = Point3f(-_radius, -_radius, 0);
        Point3f p2 = Point3f(_radius, _radius, _height);
        return Bounds3f(p1, p2);
    }

    virtual bool intersect(const Ray &ray, Float *tHit, SurfaceInteraction *isect, bool testAlphaTexture) const;

    virtual bool intersectP(const Ray &ray, bool testAlphaTexture) const;

    virtual Float area() const {
        return _radius * std::sqrt((_height * _height) + (_radius * _radius)) * _phiMax / 2;
    }

    virtual Interaction sampleA(const Point2f &u, Float *pdf) const;
    
protected:

    const Float _radius, _height, _phiMax;
};

PALADIN_END

#endif /* cone_hpp */
