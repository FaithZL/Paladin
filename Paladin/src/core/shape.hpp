//
//  shape.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef shape_hpp
#define shape_hpp

#include "core/header.h"
#include "math/transform.hpp"
#include "core/interaction.hpp"
#include "core/cobject.h"
#include "tools/embree_util.hpp"

PALADIN_BEGIN

/**
 * 所有图形的基类，假设读者已经掌握概率论基础知识
 * 科普一下，pdf为概率密度函数，pdf在定义域上积分为1，且恒不小于零
 * 如果没有读过概率密度函数，建议阅读浙大的概率密度函数，经典
 * shape类只是一个形状，不是一个具体的物体，具体的物体是primitive的子类，有材质，形状，是否发光等属性
 */
class Shape : public EmbreeUtil::EmbreeGeomtry {
public:
	Shape(const shared_ptr<const Transform> &ObjectToWorld, const shared_ptr<const Transform>&WorldToObject,
          bool reverseOrientation);

	virtual ~Shape();

	// 返回在对象坐标系中的包围盒
	virtual AABB3f objectBound() const = 0;

	// 返回在世界坐标系中的包围盒
	virtual AABB3f worldBound() const;
    
    // 初始化函数，每个子类构造时都要调用
    // 目前用于计算表面积
    virtual void init() = 0;
    
    void setPrimitive(Primitive *primitive) {
        _primitive = primitive;
    }
    
    Primitive * getPrimitive() {
        return _primitive;
    }

	// 求交函数，填充SurfaceInteraction数据
    // 几乎所有的shape与ray求交的计算都是将ray转换到object空间中进行的
	virtual bool intersect(const Ray &ray, 
							Float *tHit, 
							SurfaceInteraction * isect, 
							bool testAlphaTexture = true) const = 0;

	virtual bool intersectP(const Ray &ray,
                            bool testAlphaTexture = true) const {
        return intersect(ray, nullptr, nullptr, testAlphaTexture);
    }
    
    virtual bool fillSurfaceInteraction(const Ray &r, const Vector2f &uv, SurfaceInteraction *isect) const {
        DCHECK(false);
        return true;
    }

    // 表面积
    virtual Float area() const = 0;

    /**
     * 在图形表面采样一个点，返回该点的基于面积的概率密度函数值，与世界坐标中interaction结构
     * u为表面参数坐标，u ∈ [0, 1]^2，通常为外部通过某种算法生成，最简单就是随机数
     * 基于面积的采样
     */
    virtual Interaction samplePos(const Point2f &u, Float *pdf) const = 0;

    // 概率密度函数，表面某点的pdf，函数空间为表面参数空间
    inline Float pdfPos(const Interaction &isect) const {
        DCHECK(_invArea != 0);
        return _invArea;
    }

    // 概率密度函数，表面某点的pdf，函数空间为表面参数空间
    inline Float pdfPos() const {
        DCHECK(_invArea != 0);
    	return _invArea;
    }

    /**
	 * 在场景的某处ref处，随机采样shape，返回世界坐标中interaction结构
	 * 以及在ref处采样到对应shape表面点的概率密度函数
	 * 基于立体角的采样
     */
    virtual Interaction sampleDir(const Interaction &ref, const Point2f &u, Float *pdf) const;

	/**
	 * 在场景中某处ref处沿着wi方向采样图形上某点的概率密度函数
	 * 函数空间为向量空间
	 */
    virtual Float pdfDir(const Interaction &ref, const Vector3f &wi) const;
    
    /**
     * 返回shape对于某个点的立体角大小
     */
    virtual Float solidAngle(const Point3f &p, int nSamples = 512) const;
    
    PALADIN_INLINE void computeScatteringFunctions(SurfaceInteraction *isect,
                                    MemoryArena &arena, TransportMode mode,
                                    bool allowMultipleLobes) const {
        
    }
    
    PALADIN_INLINE const Material * getMaterial(int idx = 0) const {
        return _materials[idx].get();
    }
    
    PALADIN_INLINE const AreaLight * getAreaLight() {
        return _areaLight.get();
    }

    shared_ptr<const Transform> objectToWorld;
    shared_ptr<const Transform> worldToObject;
    // 通常是模型文件指定的属性
    const bool reverseOrientation;
    const bool transformSwapsHandedness;
protected:
    // 参照mitsuba渲染器，后续优化，保存表面积的倒数，每次图形有变化时更新数据
    // 初始值为零，0为非法值
    Float _invArea = 0;
    
    vector<shared_ptr<const Material>> _materials;
    
    // 发光属性
    std::shared_ptr<AreaLight> _areaLight;
    MediumInterface _mediumInterface;
    
    Primitive * _primitive;
};

PALADIN_END

#endif /* shape_hpp */
