//
//  disk.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "shapes/disk.hpp"
#include "math/sampling.hpp"

PALADIN_BEGIN

bool Disk::intersectP(const paladin::Ray &r, bool testAlphaTexture) const {
    Vector3f oErr, dErr;
    Ray ray = worldToObject->exec(r, &oErr, &dErr);
    if (ray.dir.z == 0) {
        return false;
    }

    Float tShapeHit = (_height - ray.ori.z) / ray.dir.z;
    if (tShapeHit <= 0 || tShapeHit >= ray.tMax) {
        return false;
    }
    
    Point3f pHit = ray.at(tShapeHit);
    Float dist2 = pHit.x * pHit.x + pHit.y * pHit.y;
    if (dist2 > _radius * _radius || dist2 < _innerRadius * _innerRadius) {
        return false;
    }
    
    Float phi = std::atan2(pHit.y, pHit.x);
    if (phi < 0) {
        phi += 2 * Pi;
    }

    if (phi > _phiMax) {
        return false;
    }
    return true;
}

bool Disk::intersect(const paladin::Ray &r, Float *tHit, paladin::SurfaceInteraction *isect, bool testAlphaTexture) const {
    Vector3f oErr, dErr;
    Ray ray = worldToObject->exec(r, &oErr, &dErr);
    if (ray.dir.z == 0) {
        return false;
    }
    
    Float tShapeHit = (_height - ray.ori.z) / ray.dir.z;
    if (tShapeHit <= 0 || tShapeHit >= ray.tMax) {
        return false;
    }
    
    Point3f pHit = ray.at(tShapeHit);
    Float dist2 = pHit.x * pHit.x + pHit.y * pHit.y;
    if (dist2 > _radius * _radius || dist2 < _innerRadius * _innerRadius) {
        return false;
    }
    
    Float phi = std::atan2(pHit.y, pHit.x);
    if (phi < 0) {
        phi += 2 * Pi;
    }
    
    if (phi > _phiMax) {
        return false;
    }
    
    // 计算微分几何信息
    Float u = phi / _phiMax;
    Float rHit = sqrtf(dist2);
    Float v = (_radius - rHit) / (_radius - _innerRadius);
    
    Vector3f dpdu(-_phiMax * pHit.y, _phiMax * pHit.x, 0);
    Vector3f dpdv = Vector3f(pHit.x, pHit.y, 0.) * (_innerRadius - _radius) / rHit;
    Normal3f dndu(0, 0, 0), dndv(0, 0, 0);
    
    pHit.z = _height;
    
    Vector3f pError(0, 0, 0);
    
    *isect = objectToWorld->exec(SurfaceInteraction(pHit, pError, Point2f(u, v), ray.dir, dpdu, dpdv, dndu, dndv, ray.time, this));
    
    *tHit = (Float)tShapeHit;
    return false;
}

Interaction Disk::samplePos(const Point2f &u, Float *pdf) const {
    Interaction ret;
    Point2f p2 = uniformSampleDisk(u);
    Point3f pObj = Point3f(p2.x * _radius, p2.y * _radius, _height);
    ret.normal = normalize(objectToWorld->exec(Normal3f(0, 0, 1)));
    if (reverseOrientation) {
        ret.normal *= -1;
    }
    ret.pos = objectToWorld->exec(pObj, Vector3f(0,0,0), &ret.pError);
    *pdf = pdfPos();
    return ret;
}

PALADIN_END
