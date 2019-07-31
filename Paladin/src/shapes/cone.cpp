//
//  cone.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "cone.hpp"
#include "sampling.hpp"

PALADIN_BEGIN

bool Cone::intersect(const paladin::Ray &ray, Float *tHit, paladin::SurfaceInteraction *isect, bool testAlphaTexture) const {
    // todo
    return true;
}
// todo
bool Cone::intersectP(const paladin::Ray &r, bool testAlphaTexture) const {
    Float phi;
    Point3f pHit;
    Vector3f oErr, dErr;
    Ray ray = worldToObject->exec(r, &oErr, &dErr);
    
    //  ((hx)/r)^2 + ((hx)/r)^2 - (z - h)^2 = 0
    //  
    EFloat ox(ray.ori.x, oErr.x), oy(ray.ori.y, oErr.y), oz(ray.ori.z, oErr.z);
    EFloat dx(ray.dir.x, dErr.x), dy(ray.dir.y, dErr.y), dz(ray.dir.z, dErr.z);
    EFloat k = EFloat(_radius) / EFloat(_height);
    k = k * k;
    EFloat a = dx * dx + dy * dy - k * dz * dz;
    EFloat b = 2 * (dx * ox + dy * oy - k * dz * (oz - _height));
    EFloat c = ox * ox + oy * oy - k * (oz - _height) * (oz - _height);
    
    return true;
}

// 暂时只实现完整圆锥的采样 ，后续优化 todo
Interaction Cone::sampleA(const Point2f &u, Float *pdf) const {
	Interaction ret;
	// R = sqrt(h^2 + r^2)
	// θmax = r/R * 2π
	Float R = std::sqrt(_height * _height + _radius * _radius);
	Float thetaMax = (_radius / R) * _2Pi;
	// 采样得到单位扇形上的点
	Point2f p2 = uniformSampleSector(u, thetaMax);
    Vector2f vec2(p2);
	// 映射到对应扇形
	p2.x = R * p2.x;
	p2.y = R * p2.y;

	Point3f pObj;
	Normal3f nObj;
    Vector3f pObjError;
	if (p2.x == 0 && p2.y == 0) {
		// 如果生成的点在圆锥顶点
		pObj = Point3f(0, 0, _height);
		nObj = Normal3f(0, 0, 1);
        pObjError = Vector3f(0, 0, 0);
	} else {
        // θ为p2在展开扇形中的夹角
        Float theta = std::atan2(p2.y, p2.x);
        Float u = theta / thetaMax;
        Float v = 1 - vec2.length();
        Float phi = u * _phiMax;
        pObj.x = _radius * (1 - v) * std::cos(phi);
        pObj.y = _radius * (1 - v) * std::sin(phi);
        pObj.z = v * _height;
        Float rh = std::sqrt(pObj.x * pObj.x + pObj.y * pObj.y);
        // 采样时由于三角函数误差，校正一把
        pObj.x *= _radius / rh;
        pObj.y *= _radius / rh;
        // 此处参照disk采样，有空再推到一把 todo
        pObjError = gamma(4) * abs(Vector3f(pObj));
	}
	// 转为世界空间
	ret.pos = objectToWorld->exec(pObj, pObjError, &ret.pError);
	ret.normal = objectToWorld->exec(nObj);
	if (reverseOrientation) {
		ret.normal *= -1;
	}
	*pdf = pdfA();
    return ret;
}

PALADIN_END
