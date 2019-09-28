//
//  interaction.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/7/11.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef interaction_hpp
#define interaction_hpp

#include "core/header.h"
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
    
    const Medium * getMedium(const Vector3f &w) const {
        return dot(w, normal) > 0 ? mediumInterface.outside : mediumInterface.inside;
    }
    
    const Medium * getMedium() const {
        CHECK_EQ(mediumInterface.inside, mediumInterface.outside);
        return mediumInterface.inside;
    }

	bool isSurfaceInteraction() const {
		return !normal.isZero();
	}

	bool isMediumInteraction() const {
		return !isSurfaceInteraction(); 
	}

    /*
     一般用于在指定交点处根据序列生成ray找到下一个交点
     */
    Ray spawnRay(const Vector3f &d) const {
        Point3f o = offsetRayOrigin(pos, pError, normal, d);
        return Ray(o, d, Infinity, time, getMedium(d));
    }
    
    /*
     一般用于生成shadow ray，判断两个点之间是否有阻挡
     */
    Ray spawnRayTo(const Point3f &p2) const {
        Point3f origin = offsetRayOrigin(pos, pError, normal, p2 - pos);
        Vector3f d = p2 - pos;
        // 这里的tMax为1，因为真实的长度已经在d方向里保存，又因为光线的终点不能在p2点上，所以。。。
        return Ray(origin, d, 1 - ShadowEpsilon, time, getMedium(d));
    }
    
    /*
     一般用于生成shadow ray，判断两个点之间是否有阻挡
     */    
    Ray spawnRayTo(const Interaction &it) const {
        Point3f origin = offsetRayOrigin(pos, pError, normal, it.pos - pos);
        Point3f target = offsetRayOrigin(it.pos, it.pError, it.normal, origin - it.pos);
        Vector3f d = target - origin;
        return Ray(origin, d, 1 - ShadowEpsilon, time, getMedium(d));
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

class MediumInteraction : public Interaction {
public:
    MediumInteraction() : phase(nullptr) {

    }

    MediumInteraction(const Point3f &p, const Vector3f &wo, Float time,
                      const Medium *medium, const PhaseFunction *phase)
    : Interaction(p, wo, time, medium), phase(phase) {

    }

    bool isValid() const { 
        return phase != nullptr; 
    }
    
    const PhaseFunction *phase;
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
