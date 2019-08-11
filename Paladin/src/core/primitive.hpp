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
    virtual ~Primitive();
    virtual AABB3f WorldBound() const = 0;
    virtual bool intersect(const Ray &r, SurfaceInteraction *) const = 0;
    virtual bool intersectP(const Ray &r) const = 0;
    virtual const AreaLight *getAreaLight() const = 0;
    virtual const Material *getMaterial() const = 0;
    virtual void computeScatteringFunctions(SurfaceInteraction *isect,
                                            MemoryArena &arena,
                                            TransportMode mode,
                                            bool allowMultipleLobes) const = 0;
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
