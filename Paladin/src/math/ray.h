//
//  ray.h
//  Paladin
//
//  Created by SATAN_Z on 2019/7/6.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef ray_h
#define ray_h

#include "header.h"

PALADIN_BEGIN

// Ray Declarations
class Ray {
public:
    // Ray Public Methods
    Ray() : tMax(Infinity), time(0.f), medium(nullptr) {
        
    }
    
    Ray(const Point3f &o, const Direction3f &d, Float tMax = Infinity,
        Float time = 0.f, const Medium *medium = nullptr)
    : o(o), d(d), tMax(tMax), time(time), medium(medium) {
        
    }
    
    Point3f operator()(Float t) const {
        return o + d * t;
    }
    
    bool hasNaNs() const {
        return (o.hasNaNs() || d.hasNaNs() || isNaN(tMax));
    }
    
    friend std::ostream &operator<<(std::ostream &os, const Ray &r) {
        os << "[o=" << r.o << ", d=" << r.d << ", tMax=" << r.tMax
        << ", time=" << r.time << "]";
        return os;
    }
    
    // Ray Public Data
    Point3f o;
    Direction3f d;
    mutable Float tMax;
    Float time;
    const Medium *medium;
};

class RayDifferential : public Ray {
public:
    // RayDifferential Public Methods
    RayDifferential() {
        hasDifferentials = false;
    }
    RayDifferential(const Point3f &o, const Direction3f &d, Float tMax = Infinity,
                    Float time = 0.f, const Medium *medium = nullptr)
    : Ray(o, d, tMax, time, medium) {
        hasDifferentials = false;
    }
    RayDifferential(const Ray &ray) : Ray(ray) {
        hasDifferentials = false;
    }
    bool HasNaNs() const {
        return Ray::hasNaNs() ||
                (hasDifferentials &&
                 (rxOrigin.hasNaNs() || ryOrigin.hasNaNs() ||
                  rxDirection.hasNaNs() || ryDirection.hasNaNs()));
    }
    void ScaleDifferentials(Float s) {
        rxOrigin = o + (rxOrigin - o) * s;
        ryOrigin = o + (ryOrigin - o) * s;
        rxDirection = d + (rxDirection - d) * s;
        ryDirection = d + (ryDirection - d) * s;
        rxDirection.normalize();
        ryDirection.normalize();
    }
    
    friend std::ostream &operator<<(std::ostream &os, const RayDifferential &r) {
        os << "[ " << (Ray &)r << " has differentials: " <<
        (r.hasDifferentials ? "true" : "false") << ", xo = " << r.rxOrigin <<
        ", xd = " << r.rxDirection << ", yo = " << r.ryOrigin << ", yd = " <<
        r.ryDirection;
        return os;
    }
    
    // RayDifferential Public Data
    bool hasDifferentials;
    Point3f rxOrigin, ryOrigin;
    Direction3f rxDirection, ryDirection;
};

PALADIN_END

#endif /* ray_h */
