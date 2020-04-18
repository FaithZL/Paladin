//
//  embree_util.hpp
//  Paladin
//
//  Created by Zero on 2020/4/5.
//

#ifndef embree_util_hpp
#define embree_util_hpp

#include "core/header.h"
#include <embree3/rtcore.h>
#include "core/cobject.h"

PALADIN_BEGIN

namespace EmbreeUtil {

void initDevice();
RTCDevice getDevice();

class EmbreeGeomtry : public CObject {
    
public:
    virtual Shape * getShape(int primID) const {
        return (Shape *)this;
    }
    
    virtual EmbreeGeomtry * getEmbreeGeomtry() const {
        return (EmbreeGeomtry *)this;
    }
    
    virtual RTCGeometry rtcGeometry(Scene * scene) const {
        DCHECK(false);
        return nullptr;
    }
    
    // 用于构造实例化Scene对象
    virtual RTCScene rtcScene(Scene * scene) const {
        return nullptr;
    }
    
};

inline RTCBounds convert(const AABB3f &b) {
    return RTCBounds{
        b.pMin.x, b.pMin.y, b.pMin.z, 0.0f,
        b.pMax.x, b.pMax.y, b.pMax.z, 0.0f
    };
}

inline AABB3f convert(const RTCBounds &b) {
    return AABB3f(
        Point3f(b.lower_x, b.lower_y, b.lower_z),
        Point3f(b.upper_x, b.upper_y, b.upper_z)
    );
}

inline Ray convert(const RTCRay &r) {
    Point3f org(r.org_x, r.org_y, r.org_z);
    Vector3f dir(r.dir_x, r.dir_y, r.dir_z);
    Ray ret = Ray(org, dir, r.tfar);
    ret.tMin = r.tnear;
    return ret;
}

inline RTCRay convert(const Ray &r) {
    RTCRay ray;
    ray.org_x = r.ori.x;
    ray.org_y = r.ori.y;
    ray.org_z = r.ori.z;
    ray.dir_x = r.dir.x;
    ray.dir_y = r.dir.y;
    ray.dir_z = r.dir.z;
    ray.tnear = r.tMin;
    ray.tfar  = r.tMax;
    ray.flags = 0;
    return ray;
}

inline RTCRayHit toRTCRayHit(const Ray &r) {
    auto ray = convert(r);
    RTCRayHit rh;
    rh.ray = ray;
    rh.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    rh.hit.primID = RTC_INVALID_GEOMETRY_ID;
    return rh;
}

}

PALADIN_END

#endif /* embree_util_hpp */
