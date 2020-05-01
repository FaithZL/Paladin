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
#include "core/material.hpp"
#include "math/frame.hpp"

PALADIN_BEGIN

/**
 * Interaction为光线与物体作用的点
 * 该结构储存着各种数据
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

    /**
     * 一般用于在指定交点处根据序列生成ray找到下一个交点
     */
    Ray spawnRay(const Vector3f &d) const;
    
    /**
     * 一般用于生成shadow ray，判断两个点之间是否有阻挡
     */
    Ray spawnRayTo(const Point3f &p2) const;
    
    /**
     * 一般用于生成shadow ray，判断两个点之间是否有阻挡
     */    
    Ray spawnRayTo(const Interaction &it) const;

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
	SurfaceInteraction() {
        
    }
    
	SurfaceInteraction(const Point3f &p, const Vector3f &pError,
                       const Point2f &uv, const Vector3f &wo,
                       const Vector3f &dpdu, const Vector3f &dpdv,
                       const Normal3f &dndu, const Normal3f &dndv, Float time,
                       const Shape * sh,
                       int faceIndex = 0);

	void setShadingGeometry(const Vector3f &dpdu, const Vector3f &dpdv,
                            const Normal3f &dndu, const Normal3f &dndv,
                            bool orientationIsAuthoritative);
    
    /**
     * 计算偏导数 du/dx, du/dy, dv/dx, dv/dy
     * 光线在x方向变化时，对应纹理的u，v坐标的变化率
     * x，y方向偏移值通常为1/√(spp)，这还是比较容易理解的
     * 一个像素采spp个样本，相当于把一个像素平均分为spp份，xy方向各为√(spp)
     * 
     * 这些数据主要用于纹理反走样，把相邻两个采样点之间的变化值过滤掉
     * @param ray [description]
     */
    void computeDifferentials(const RayDifferential &ray) const;

    void computeScatteringFunctions(const RayDifferential &ray,
                                    MemoryArena &arena,
                                    bool allowMultipleLobes = false,
                                    TransportMode mode = TransportMode::Radiance);
    
    void computeTangentSpace();
    
    inline void faceForward() {
        normal = faceforward(normal, shading.normal);
    }
    
    Spectrum Le(const Vector3f &w) const;
	
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
    
    mutable Frame tangentSpace;

    /*
    表面坐标对屏幕坐标对屏幕坐标的一阶导数
    dudx为 屏幕坐标x变化时导致表面参数坐标u的变化产生的变化率，其他同理
     */
    mutable Float dudx = 0, dvdx = 0, dudy = 0, dvdy = 0;

    int faceIndex = 0;
};

PALADIN_END

#endif /* interaction_hpp */
