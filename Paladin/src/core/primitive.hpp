//
//  primitive.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/8/10.
//

#ifndef primitive_hpp
#define primitive_hpp

#include "header.h"
#include "interaction.hpp"
#include "material.hpp"

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
    const AreaLight *getAreaLight() const;
    const Material *getMaterial() const;
    void computeScatteringFunctions(SurfaceInteraction *isect,
                                    MemoryArena &arena, TransportMode mode,
                                    bool allowMultipleLobes) const;
    
private:
    
    std::shared_ptr<Shape> _shape;
    std::shared_ptr<Material> _material;
    // 发光属性
    std::shared_ptr<AreaLight> _areaLight;
    MediumInterface _mediumInterface;
};

class Aggregate : public Primitive {
public:

    const AreaLight *getAreaLight() const {
        DCHECK(false);
        return nullptr;
    }
    const Material *getMaterial() const {
        DCHECK(false);
        return nullptr;
    }
    void computeScatteringFunctions(SurfaceInteraction *isect,
                                    MemoryArena &arena, TransportMode mode,
                                    bool allowMultipleLobes) const {
        DCHECK(false);
    }
};

PALADIN_END

#endif /* primitive_hpp */
