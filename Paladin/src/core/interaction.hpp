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
    mediumInterface(mediumInterface) {

	}
    
    Interaction(const Point3f &p, const Vector3f &wo, Float time,
                const MediumInterface &mediumInterface)
    : pos(p),
    time(time),
    wo(wo),
    mediumInterface(mediumInterface) {
        
    }
    
    Interaction(const Point3f &p, Float time,
                const MediumInterface &mediumInterface)
    : pos(p),
    time(time),
    mediumInterface(mediumInterface) {
        
    }
    
    const Medium * GetMedium(const Vector3f &w) const {
        return dot(w, normal) > 0 ? mediumInterface.outside : mediumInterface.inside;
    }
    
    const Medium * GetMedium() const {
        CHECK_EQ(mediumInterface.inside, mediumInterface.outside);
        return mediumInterface.inside;
    }

	bool isSurfaceInteraction() const {
		return !normal.isZero();
	}

	bool isMediumInteraction() const {
		return !isSurfaceInteraction(); 
	}

    Ray spawnRay(const Vector3f &d) const {
        /*
          由于计算出的交点可能会有误差，如果直接把pos作为光线的起点，可能取到的是shape内部的点
          如果从内部的点发出光线，则可能产生自相交，为了避免这种情况，通常会对pos做一定的偏移
         */
        Point3f o = offsetRayOrigin(pos, pError, normal, d);
        return Ray(o, d, Infinity, time, GetMedium(d));
    }
    
    Ray spawnRayTo(const Point3f &p2) const {
        Point3f origin = offsetRayOrigin(pos, pError, normal, p2 - pos);
        Vector3f d = p2 - pos;
        return Ray(origin, d, 1 - ShadowEpsilon, time, GetMedium(d));
    }
    
    Ray spawnRayTo(const Interaction &it) const {
        Point3f origin = offsetRayOrigin(pos, pError, normal, it.pos - pos);
        Point3f target = offsetRayOrigin(it.pos, it.pError, it.normal, origin - it.pos);
        Vector3f d = target - origin;
        return Ray(origin, d, 1 - ShadowEpsilon, time, GetMedium(d));
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
                       const Shape * sh,
                       int faceIndex = 0);

	void setShadingGeometry(const Vector3f &dpdu, const Vector3f &dpdv,
                            const Normal3f &dndu, const Normal3f &dndv,
                            bool orientationIsAuthoritative);
	
    // 表面坐标
	Point2f uv;
    /*
     空间点对表面坐标的一阶导数
     当u变化时，表面交点p随u变化的变化率，v同理
     */
    Vector3f dpdu, dpdv;

    /*
     空间点对表面坐标的一阶导数
     (当u变化时，法线n随u变化的变化率，v同理)
     */
    Normal3f dndu, dndv;
    
    const Shape * shape = nullptr;
    Shading shading;
    const Primitive * primitive = nullptr;
    BSDF * bsdf = nullptr;
    BSSRDF * bssrdf = nullptr;
    
    /*
    空间点对屏幕坐标之间的一阶导数
    当屏幕坐标x变化时，表面交点p随x的变化率，y同理
     */
    mutable Vector3f dpdx, dpdy;

    /*
    表面坐标对屏幕坐标对屏幕坐标的一阶导数
    dudx为 屏幕坐标x变化时导致表面参数坐标u的变化产生的变化率，其他同理
     */
    mutable Float dudx = 0, dvdx = 0, dudy = 0, dvdy = 0;

    int faceIndex = 0;
};

PALADIN_END

#endif /* interaction_hpp */
