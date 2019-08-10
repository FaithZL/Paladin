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


class Primitive {
public:
    virtual ~Primitive();
    virtual AABB3f WorldBound() const = 0;
    virtual bool intersect(const Ray &r, SurfaceInteraction *) const = 0;
    virtual bool intersectP(const Ray &r) const = 0;
//    virtual const AreaLight *GetAreaLight() const = 0;
    virtual const Material *getMaterial() const = 0;
    virtual void computeScatteringFunctions(SurfaceInteraction *isect,
                                            MemoryArena &arena,
                                            TransportMode mode,
                                            bool allowMultipleLobes) const = 0;
};


PALADIN_END

#endif /* primitive_hpp */
