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
    EFloat ox(ray.ori.x, oErr.x), oy(ray.ori.y, oErr.y), oz(ray.ori.z, oErr.z);
    EFloat dx(ray.dir.x, dErr.x), dy(ray.dir.y, dErr.y), dz(ray.dir.z, dErr.z);
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
        if (t1.upperBound() > ray.tMax) {
        	return false;
        }

        // 此时的逻辑是t0处于圆柱的缺失部分，则开始判断t1
        pHit = ray.at((Float)tShapeHit);

        Float hitRad = std::sqrt(pHit.x * pHit.x + pHit.y * pHit.y);
        pHit.x *= _radius / hitRad;
        pHit.y *= _radius / hitRad;
        phi = std::atan2(pHit.y, pHit.x);
        if (phi < 0) {
        	phi += 2 * Pi;
        }

        if (pHit.z < _zMin || pHit.z > _zMax || phi > _phiMax) {
        	return false;
        }
    }

    // 计算微分几何信息
    Float u = phi / _phiMax;
    Float v = (pHit.z - _zMin) / (_zMax - _zMin);

    Vector3f dpdu(-_phiMax * pHit.y, _phiMax * pHit.x, 0);
    Vector3f dpdv(0, 0, _zMax - _zMin);

    Vector3f d2Pduu = -_phiMax * _phiMax * Vector3f(pHit.x, pHit.y, 0);
    Vector3f d2Pduv(0, 0, 0), d2Pdvv(0, 0, 0);

    Float E = dot(dpdu, dpdu);
    Float F = dot(dpdu, dpdv);
    Float G = dot(dpdv, dpdv);
    Vector3f N = normalize(cross(dpdu, dpdv));
    Float e = dot(N, d2Pduu);
    Float f = dot(N, d2Pduv);
    Float g = dot(N, d2Pdvv);

    Float invEGF2 = 1 / (E * G - F * F);
    Normal3f dndu = Normal3f((f * F - e * G) * invEGF2 * dpdu +
                             (e * F - f * E) * invEGF2 * dpdv);
    Normal3f dndv = Normal3f((g * F - f * G) * invEGF2 * dpdu +
                             (f * F - g * E) * invEGF2 * dpdv);
    /*
     x' = x * (r / sqrt(x^2 + y^2))
     x' = x ⊗ r ⊘ sqrt((x ⊗ x) ⊕ (y ⊗ y))
     ∈ xr(1± ε)^2 / sqrt(x^2*(1 ± ε)^2 + y^2*(1 ± ε)^2)(1 ± ε)
     ∈ xr(1± γ2) / sqrt(x^2*(1 ± γ2) + y^2*(1 ± γ2))(1 ± γ1)
     ∈ xr(1± γ2) / sqrt(x^2* + y^2) * (1 ± γ2)
     ∈ (xr / sqrt(x2 + y2)) * (1 ± γ4)
     这里我自己算得gamma(4)，pbrt的结果为3，暂时不理解，先按照自己的思路写
     y同理
    */
    Vector3f pError = gamma(4) * abs(Vector3f(pHit.x, pHit.y, 0));

    *isect = objectToWorld->exec(SurfaceInteraction(pHit, pError, Point2f(u, v),
                                                 -ray.dir, dpdu, dpdv, dndu, dndv,
                                                    ray.time, this));

    *tHit = (Float)tShapeHit;
    return true;
}

bool Cylinder::intersectP(const Ray &r, bool testAlphaTexture) const {
	Float phi;
    Point3f pHit;

    Vector3f oErr, dErr;
    Ray ray = worldToObject->exec(r, &oErr, &dErr);
    // 圆柱方程为：x^2 + y^2 − r^2 = 0
    // 带入ray上的点：(ox + t * dx)^2 + (oy + t * dy)^2 - r^2 = 0
    // a * t^2 + b * t + c = 0，则a，b，c如下所示
    EFloat ox(ray.ori.x, oErr.x), oy(ray.ori.y, oErr.y), oz(ray.ori.z, oErr.z);
    EFloat dx(ray.dir.x, dErr.x), dy(ray.dir.y, dErr.y), dz(ray.dir.z, dErr.z);
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
        if (t1.upperBound() > ray.tMax) {
        	return false;
        }

        // 此时的逻辑是t0处于圆柱的缺失部分，则开始判断t1
        pHit = ray.at((Float)tShapeHit);

        Float hitRad = std::sqrt(pHit.x * pHit.x + pHit.y * pHit.y);
        pHit.x *= _radius / hitRad;
        pHit.y *= _radius / hitRad;
        phi = std::atan2(pHit.y, pHit.x);
        if (phi < 0) {
        	phi += 2 * Pi;
        }

        if (pHit.z < _zMin || pHit.z > _zMax || phi > _phiMax) {
        	return false;
        }
    }
    return true;
}

Interaction Cylinder::samplePos(const Point2f &u, Float *pdf) const {
	Float z = lerp(u[0], _zMin, _zMax);
	Float phi = u[1] * _phiMax;

	Point3f pObj = Point3f(_radius * std::cos(phi), _radius * std::sin(phi), z);
	Interaction ret;
	ret.normal = normalize(objectToWorld->exec(Normal3f(pObj.x, pObj.y, 0)));
	if (reverseOrientation) {
		ret.normal *= -1;
	}
	Float hitRad = std::sqrt(pObj.x * pObj.x + pObj.y * pObj.y);
    pObj.x *= _radius / hitRad;
    pObj.y *= _radius / hitRad;

    // 推导过程在intersect函数中如上所示
    Vector3f pObjError = gamma(4) * abs(Vector3f(pObj.x, pObj.y, 0));
    ret.pos = objectToWorld->exec(pObj, pObjError, &ret.pError);
    *pdf = pdfPos();
    return ret;
}

//"param" : {
//    "transform" : {
//        "type" : "translate",
//        "param" : [-1,1,1]
//    },
//    "radius" : 1,
//    "phiMax" : 360,
//    "reverseOrientation" : false
//    "zMin" : -0.5,
//    "zMax" : 0.5
//}
CObject_ptr createCylinder(const nloJson &param, const Arguments &lst){
    nloJson l2w_data = param.value("transform", nloJson());
    Transform * l2w = createTransform(l2w_data);
    auto w2o(l2w->getInverse_ptr());
    auto o2w(l2w);
    
    Float radius = param.value("radius", 1.f);
    Float phiMax = param.value("phiMax", 360.f);
    Float reverseOrientation = param.value("reverseOrientation", false);
    Float zMin = param.value("zMin", -radius);
    Float zMax = param.value("zMax", radius);
    
    return new Cylinder(o2w, w2o, reverseOrientation, radius, zMin, zMax, phiMax);
}

REGISTER("cylinder", createCylinder)

PALADIN_END
