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
#include "medium.hpp"

PALADIN_BEGIN

/*
Interaction为光线与物体作用的点
该结构储存着各种数据
*/
struct Interaction {
    
	Interaction(): time(0) {}

	Interaction(const Point3f &p, const Normal3f &n, const Vector3f &pError,
                const Vector3f &wo, Float time,
                const MediumInterface &mediumInterface)
	:pos(p),
	time(time),
	pError(pError),
	wo(normalize(wo)),
	normal(n),
    mediumInterface(mediumInterface)
    {

	}

	bool isSurfaceInteraction() const {
		return !normal.isZero();
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

    Point3f pos;

    Float time;

    // 位置误差
    Vector3f pError;

    // 出射方向
    Vector3f wo;

    Normal3f normal;

    MediumInterface mediumInterface;
};


class SurfaceInteraction : public Interaction {
    // 用于着色的参数结构
    struct Shading {
        Normal3f normal;
        Vector3f dpdu, dpdv;
        Normal3f dndu, dndv;
    };
    
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
	
    // 表面坐标
	Point2f uv;
    // 空间点对表面坐标的一阶导数
    Vector3f dpdu, dpdv;
    // 法线对表面坐标的一阶导数
    Normal3f dndu, dndv;
    
    const Shape *shape = nullptr;
    Shading shading;
    const Primitive * primitive = nullptr;
    BSDF * bsdf = nullptr;
    BSSRDF * bssrdf = nullptr;
    
    // 空间点对屏幕坐标之间的一阶导数
    mutable Vector3f dpdx, dpdy;
    // 表面坐标对屏幕坐标对屏幕坐标的一阶导数
    mutable Float dudx = 0, dvdx = 0, dudy = 0, dvdy = 0;

    int faceIndex = 0;
};

PALADIN_END

#endif /* interaction_hpp */
