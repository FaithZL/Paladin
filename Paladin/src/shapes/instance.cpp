//
//  instance.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/5/17.
//

#include "instance.hpp"


PALADIN_BEGIN


void Instance::initial() {
    // todo
}

bool Instance::rayIntersect(const Ray &r,
                            SurfaceInteraction * isect,
                            bool testAlphaTexture) const {
    auto ray = worldToObject->exec(r);
    if (!_shape->rayIntersect(ray, isect, testAlphaTexture)) {
        return false;
    }
    r.tMax = ray.tMax;
    
    if (!objectToWorld->isIdentity()) {
        *isect = objectToWorld->exec(*isect);
    }
    
    if (_mediumInterface.isMediumTransition()){
        isect->mediumInterface = _mediumInterface;
    } else {
        isect->mediumInterface = MediumInterface(r.medium);
    }
    isect->shape = this;
    return true;
}

RTCGeometry Instance::rtcGeometry(Scene * scene) const {
    RTCGeometry ret = rtcNewGeometry(EmbreeUtil::getDevice(), RTC_GEOMETRY_TYPE_INSTANCE);
    
    return ret;
}

bool Instance::rayOccluded(const Ray &r, bool testAlphaTexture) const {
    auto ray = worldToObject->exec(r);
    return _shape->rayOccluded(ray);
}

PALADIN_END
