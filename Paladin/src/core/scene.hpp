//
//  scene.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef scene_hpp
#define scene_hpp

#include "core/header.h"
#include "core/primitive.hpp"
#include "core/light.hpp"


PALADIN_BEGIN

class Scene {
public:
    Scene(std::shared_ptr<Primitive> aggregate,
          const std::vector<std::shared_ptr<Light>> &lights)
    : lights(lights), _aggregate(aggregate) {
        // Scene Constructor Implementation
        _worldBound = _aggregate->worldBound();
        for (const auto &light : lights) {
            light->preprocess(*this);
            if (light->flags & (int)LightFlags::Infinite)
                infiniteLights.push_back(light);
        }
    }
    const AABB3f &worldBound() const { return _worldBound; }
    bool intersect(const Ray &ray, SurfaceInteraction *isect) const;
    bool intersectP(const Ray &ray) const;
    bool intersectTr(Ray ray, Sampler &sampler, SurfaceInteraction *isect,
                     Spectrum *transmittance) const;
    
    // Scene Public Data
    std::vector<std::shared_ptr<Light>> lights;
    // Store infinite light sources separately for cases where we only want
    // to loop over them.
    std::vector<std::shared_ptr<Light>> infiniteLights;
    
private:
    // Scene Private Data
    std::shared_ptr<Primitive> _aggregate;
    AABB3f _worldBound;
};

PALADIN_END

#endif /* scene_hpp */
