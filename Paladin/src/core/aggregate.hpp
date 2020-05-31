//
//  aggregate.hpp
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
#include "tools/embree_util.hpp"

PALADIN_BEGIN


class Aggregate : public CObject {
public:

    virtual AABB3f worldBound() const = 0;
    
    virtual bool rayIntersect(const Ray &ray, SurfaceInteraction * isect) const {
        DCHECK(false);
    }
    
    virtual bool rayOccluded(const Ray &ray) const {
        DCHECK(false);
    }
    
    virtual const AreaLight *getAreaLight() const  {
        DCHECK(false);
        return nullptr;
    }
    virtual const Material *getMaterial() const  {
        DCHECK(false);
        return nullptr;
    }
};

shared_ptr<Aggregate> createAccelerator(const nloJson &data, const vector<shared_ptr<const Shape>> &);

PALADIN_END

#endif /* aggregate_hpp */
