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
bool Cone::intersectP(const paladin::Ray &ray, bool testAlphaTexture) const {
    return true;
}


Interaction Cone::sampleA(const Point2f &u, Float *pdf) const {
	Interaction ret;
	// R = sqrt(h^2 + r^2)
	// θmax = r/R * 2π
	Float R = std::sqrt(_height * _height + _radius * _radius);
	Float thetaMax = (_radius / R) * _2Pi;
	// 采样得到单位扇形上的点
	Point2f p2 = uniformSampleSector(u, thetaMax);
	// 映射到对应扇形
	p2.x = _radius * p2.x;
	p2.y = _radius * p2.y;

	Point3f pObj;
	Normal3f nObj;
	if (p2.x == 0 && p2.y == 0) {
		// 如果生成的点在圆锥顶点
		pObj = Point3f(0, 0, _height);
		nObj = Normal3f(0, 0, 1);
	} else {
	
	}
	// 转为世界空间
	ret.pos = objectToWorld->exec(pObj, Vector3f(0,0,0), &ret.pError);
	ret.normal = objectToWorld->exec(nObj);
	if (reverseOrientation) {
		ret.normal *= -1;
	}
	*pdf = pdfA();
    return ret;
}

PALADIN_END
