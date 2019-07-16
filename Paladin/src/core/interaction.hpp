//
//  interaction.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/7/11.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef interaction_hpp
#define interaction_hpp

#include "header.h"

PALADIN_BEGIN

/*
Interaction为光线与物体作用的点
该结构储存着各种数据
*/
struct Interaction {
    
	Interaction(): _time(0) {}

	Interaction(const Point3f &p, const Normal3f &n, const Vector3f &pError,
                const Vector3f &wo, Float time,
                const MediumInterface &mediumInterface):
	:_pos(p),
	_time(time),
	_pError(pError),
	_wo(Normalize(wo)),
	_normal(n),
	_mediumInterface(mediumInterface) {

	}

	bool isSurfaceInteraction() const {
		return !_normal.isZero();
	}

	bool IsMediumInteraction() const {
		return !isSurfaceInteraction(); 
	}

	Ray spawnRayTo(const Point3f &pos) const {
		// todo
		return Ray();
	}

	Ray spawnRayTo(const Vector3f &dir) const {
		// todo
		return Ray();
	}

	Ray spawnRayTo(const Interaction &it) const {
		// todo
		return Ray();
	}

    Point3f _pos;

    Float _time;

    // 位置误差
    Vector3f _pError;

    // 出射方向
    Vector3f _wo;

    Normal3f _normal;

    MediumInterface _mediumInterface;
};


class SurfaceInteraction : public Interaction {

public:
	SurfaceInteraction() {}
	SurfaceInteraction(const Point3f &p, const Vector3f &pError,
                       const Point2f &uv, const Vector3f &wo,
                       const Vector3f &dpdu, const Vector3f &dpdv,
                       const Normal3f &dndu, const Normal3f &dndv, Float time,
                       const Shape *sh,
                       int faceIndex = 0);

	void setShadingGeometry(const Vector3f &dpdu, const Vector3f &dpdv,
                            const Normal3f &dndu, const Normal3f &dndv,
                            bool orientationIsAuthoritative);
	

	Point2f _uv;
    Vector3f _dpdu, _dpdv;
    Normal3f _dndu, _dndv;
    const Shape *_shape = nullptr;
    struct {
        Normal3f normal;
        Vector3f dpdu, dpdv;
        Normal3f dndu, dndv;
    } _shading;
    const Primitive * _primitive = nullptr;
    BSDF * _bsdf = nullptr;
    BSSRDF * _bssrdf = nullptr;
    mutable Vector3f _dpdx, _dpdy;
    mutable Float _dudx = 0, _dvdx = 0, _dudy = 0, _dvdy = 0;

    int _faceIndex = 0;
};

PALADIN_END

#endif /* interaction_hpp */
