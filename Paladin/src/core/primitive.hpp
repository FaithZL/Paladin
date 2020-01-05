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
#include "core/cobject.h"

PALADIN_BEGIN

// Primitive可以理解为片段
class Primitive : public CObject {
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
    
    GeometricPrimitive(const std::shared_ptr<Shape> &shape,
                          const std::shared_ptr<const Material> &material,
                          const std::shared_ptr<AreaLight> &areaLight,
                          const MediumInterface &mediumInterface);
    
    virtual AABB3f worldBound() const override;
    
    virtual bool intersect(const Ray &r, SurfaceInteraction *isect) const override;
    
    virtual bool intersectP(const Ray &r) const override;
    
    virtual const AreaLight *getAreaLight() const override;
    
    virtual const Material *getMaterial() const override;
    
    virtual void computeScatteringFunctions(SurfaceInteraction *isect,
                                    MemoryArena &arena, TransportMode mode,
                                    bool allowMultipleLobes) const override;
    
    virtual nloJson toJson() const override {
        return nloJson();
    }
    
    static shared_ptr<GeometricPrimitive> create(const std::shared_ptr<Shape> &shape,
                                        const std::shared_ptr<const Material> &material,
                                        const std::shared_ptr<AreaLight> &areaLight,
                                          const MediumInterface &mediumInterface);
    
private:

    
    std::shared_ptr<Shape> _shape;
    std::shared_ptr<const Material> _material;
    // 发光属性
    std::shared_ptr<AreaLight> _areaLight;
    MediumInterface _mediumInterface;
};

// 用于多个完全相同的实例，只保存一个实例对象在内存中，
// 其他的不同通过transform来区分，节省内存空间
class TransformedPrimitive : public Primitive {
public:
    TransformedPrimitive(std::shared_ptr<Primitive> &primitive,
                         const AnimatedTransform &PrimitiveToWorld,
                         const std::shared_ptr<const Material> &mat = nullptr);
    
    virtual bool intersect(const Ray &r, SurfaceInteraction *in) const override;
    
    virtual bool intersectP(const Ray &r) const override;
    
    virtual const AreaLight *getAreaLight() const override {
        return _primitive->getAreaLight();
    }
    
    virtual const Material *getMaterial() const override {
        return _material != nullptr
                ? _material.get()
                : _primitive->getMaterial();
    }
    
    virtual nloJson toJson() const override {
        return nloJson();
    }
    
    virtual void computeScatteringFunctions(SurfaceInteraction *isect,
                                    MemoryArena &arena, TransportMode mode,
                                    bool allowMultipleLobes) const override {
        LOG(FATAL) <<
        "TransformedPrimitive::ComputeScatteringFunctions() shouldn't be "
        "called";
    }
    
    virtual AABB3f worldBound() const override {
        return _primitiveToWorld.motionAABB(_primitive->worldBound());
    }
    
private:
    std::shared_ptr<Primitive> _primitive;
    const AnimatedTransform _primitiveToWorld;
    // 材质也可能不同
    std::shared_ptr<const Material> _material;
    
};

class Aggregate : public Primitive {
public:

    virtual const AreaLight *getAreaLight() const override {
        DCHECK(false);
        return nullptr;
    }
    virtual const Material *getMaterial() const override {
        DCHECK(false);
        return nullptr;
    }
    virtual void computeScatteringFunctions(SurfaceInteraction *isect,
                                    MemoryArena &arena, TransportMode mode,
                                    bool allowMultipleLobes) const override {
        DCHECK(false);
    }
};

shared_ptr<Aggregate> createAccelerator(const nloJson &data, const vector<shared_ptr<Primitive>> &);


PALADIN_END

#endif /* primitive_hpp */
