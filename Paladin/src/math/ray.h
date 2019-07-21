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
    
    Ray(const Point3f &ori, const Vector3f &dir, Float tMax = Infinity,
        Float time = 0.f, const Medium *medium = nullptr)
    : ori(ori),
    dir(dir),
    tMax(tMax),
    time(time),
    medium(medium) {
        
    }
    
    Point3f operator()(Float t) const {
        return ori + dir * t;
    }
    
    bool hasNaNs() const {
        return (ori.hasNaNs() || dir.hasNaNs() || isNaN(tMax));
    }
    
    friend std::ostream &operator<<(std::ostream &os, const Ray &r) {
        os << "[o=" << r.ori << ", d=" << r.dir << ", tMax=" << r.tMax
        << ", time=" << r.time << "]";
        return os;
    }
    
    // Ray Public Data
    Point3f ori;
    Vector3f dir;
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
    RayDifferential(const Point3f &ori, const Vector3f &dir, Float tMax = Infinity,
                    Float time = 0.f, const Medium *medium = nullptr)
    : Ray(ori, dir, tMax, time, medium) {
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
        rxOrigin = ori + (rxOrigin - ori) * s;
        ryOrigin = ori + (ryOrigin - ori) * s;
        rxDirection = dir + (rxDirection - dir) * s;
        ryDirection = dir + (ryDirection - dir) * s;
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
    Vector3f rxDirection, ryDirection;
};

inline Point3f offsetRayOrigin(const Point3f &p, const Vector3f &pError,
                               const Normal3f &n, const Vector3f &w) {
    Float d = dot(abs(n), pError);
#ifdef FLOAT_AS_DOUBLE
    d *= 1024.;
#endif
    Vector3f offset = d * Vector3f(n);
    if (dot(w, n) < 0) offset = -offset;
    Point3f po = p + offset;
    // Round offset point _po_ away from _p_
    for (int i = 0; i < 3; ++i) {
        if (offset[i] > 0)
            po[i] = nextFloatUp(po[i]);
        else if (offset[i] < 0)
            po[i] = nextFloatDown(po[i]);
    }
    return po;
}

PALADIN_END

#endif /* ray_h */
