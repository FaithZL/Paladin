//
//  cylinder.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "cylinder.hpp"

PALADIN_BEGIN

bool Cylinder::intersect(const Ray &r, Float *tHit, SurfaceInteraction *isect, bool testAlphaTexture) const {
	Float phi;
    Point3f pHit;

    Vector3f oErr, dErr;
    Ray ray = worldToObject->exec(r, &oErr, &dErr);
    // 圆柱方程为：x^2 + y^2 − r^2 = 0
    // 带入ray上的点：(ox + t * dx)^2 + (oy + t * dy)^2 - r^2 = 0
    // a * t^2 + b * t + c = 0，则a，b，c如下所示
    EFloat ox(ray.o.x, oErr.x), oy(ray.o.y, oErr.y), oz(ray.o.z, oErr.z);
    EFloat dx(ray.d.x, dErr.x), dy(ray.d.y, dErr.y), dz(ray.d.z, dErr.z);
    EFloat a = dx * dx + dy * dy;
    EFloat b = 2 * (dx * ox + dy * oy);
    EFloat c = ox * ox + oy * oy - EFloat(_radius) * EFloat(_radius);

    EFloat t0, t1;
    if (!quadratic(a, b, c, &t0, &t1)) {
    	return false;
    }

    // 保守计算
    if (t0.upperBound() > ray.tMax || t1.lowerBound() <= 0) {
         return false;
    }
    EFloat tShapeHit = t0;

    if (tShapeHit.lowerBound() <= 0) {
        tShapeHit = t1;
        if (tShapeHit.upperBound() > ray.tMax) {
            return false;
        }
    }

    pHit = ray.at((Float)tShapeHit);

    Float hitRad = std::sqrt(pHit.x * pHit.x + pHit.y * pHit.y);
    pHit.x *= _radius / hitRad;
    pHit.y *= _radius / hitRad;

    phi = std::atan2(pHit.y, pHit.x);
    if (phi < 0) {
    	phi += 2 * Pi;
    }

    if (pHit.z < _zMin || pHit.z > _zMax || phi > _phiMax) {
    	// 测试交点是否在圆柱的缺失部分
        if (tShapeHit == t1) {
        	return false;
        }

        tShapeHit = t1;
        if (t1.UpperBound() > ray.tMax) {
        	return false;
        }

        // 此时的逻辑是t0处于圆柱的缺失部分，则开始判断t1
        pHit = ray((Float)tShapeHit);

        Float hitRad = std::sqrt(pHit.x * pHit.x + pHit.y * pHit.y);
        pHit.x *= radius / hitRad;
        pHit.y *= radius / hitRad;
        phi = std::atan2(pHit.y, pHit.x);
        if (phi < 0) {
        	phi += 2 * Pi;
        }

        if (pHit.z < _zMin || pHit.z > _zMax || phi > _phiMax) {
        	return false;
        }
    }

    // 计算微分几何信息
}

PALADIN_END
