//
//  primitive.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/8/10.
//

#ifndef primitive_hpp
#define primitive_hpp

#include "core/header.h"
#include "core/interaction.hpp"
#include "core/material.hpp"
#include "math/animatedtransform.hpp"

PALADIN_BEGIN

// Primitive可以理解为片段
class Primitive {
public:
    virtual ~Primitive() {
        
    }
    virtual AABB3f worldBound() const = 0;
    virtual bool intersect(const Ray &r, SurfaceInteraction *) const = 0;
    virtual bool intersectP(const Ray &r) const = 0;
    virtual const AreaLight *getAreaLight() const = 0;
    virtual const Material *getMaterial() const = 0;
    virtual void computeScatteringFunctions(SurfaceInteraction *isect,
                                            MemoryArena &arena,
                                            TransportMode mode,
                                            bool allowMultipleLobes) const = 0;
};

// 几何片元，有形状，材质，是否发光等属性，是需要渲染的具体物件
class GeometricPrimitive : public Primitive {
    
public:
    virtual AABB3f worldBound() const;
    virtual bool intersect(const Ray &r, SurfaceInteraction *isect) const;
    virtual bool intersectP(const Ray &r) const;
    GeometricPrimitive(const std::shared_ptr<Shape> &shape,
                       const std::shared_ptr<Material> &material,
                       const std::shared_ptr<AreaLight> &areaLight,
                       const MediumInterface &mediumInterface);
    virtual const AreaLight *getAreaLight() const;
    virtual const Material *getMaterial() const;
    virtual void computeScatteringFunctions(SurfaceInteraction *isect,
                                    MemoryArena &arena, TransportMode mode,
                                    bool allowMultipleLobes) const;
    
private:
    
    std::shared_ptr<Shape> _shape;
    std::shared_ptr<Material> _material;
    // 发光属性
    std::shared_ptr<AreaLight> _areaLight;
    MediumInterface _mediumInterface;
};

// 用于多个完全相同的实例，只保存一个实例对象在内存中，其他的不同通过transform来区分，节省内存空间
class TransformedPrimitive : public Primitive {
public:
    TransformedPrimitive(std::shared_ptr<Primitive> &primitive,
                         const AnimatedTransform &PrimitiveToWorld);
    
    virtual bool intersect(const Ray &r, SurfaceInteraction *in) const;
    
    virtual bool intersectP(const Ray &r) const;
    
    virtual const AreaLight *getAreaLight() const {
        return nullptr;
    }
    
    virtual const Material *getMaterial() const {
        return nullptr;
    }
    
    virtual void computeScatteringFunctions(SurfaceInteraction *isect,
                                    MemoryArena &arena, TransportMode mode,
                                    bool allowMultipleLobes) const {
        COUT <<
        "TransformedPrimitive::ComputeScatteringFunctions() shouldn't be "
        "called";
        DCHECK(false);
    }
    
    virtual AABB3f WorldBound() const {
        return _primitiveToWorld.motionAABB(_primitive->worldBound());
    }
    
private:
    std::shared_ptr<Primitive> _primitive;
    const AnimatedTransform _primitiveToWorld;
};

class Aggregate : public Primitive {
public:

    virtual const AreaLight *getAreaLight() const {
        DCHECK(false);
        return nullptr;
    }
    virtual const Material *getMaterial() const {
        DCHECK(false);
        return nullptr;
    }
    virtual void computeScatteringFunctions(SurfaceInteraction *isect,
                                    MemoryArena &arena, TransportMode mode,
                                    bool allowMultipleLobes) const {
        DCHECK(false);
    }
};

PALADIN_END

#endif /* primitive_hpp */
