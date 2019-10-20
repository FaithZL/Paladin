//
//  integrator.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef integrator_hpp
#define integrator_hpp

#include "header.h"
#include "scene.hpp"
#include "primitive.hpp"
#include "spectrum.hpp"
#include "light.hpp"
#include "bxdf.hpp"
#include "sampler.hpp"
#include "material.hpp"

PALADIN_BEGIN

class Integrator {
    
public:
    virtual ~Integrator() {
        
    }
    virtual void render(const Scene &) = 0;
};

Spectrum uniformSampleAllLights(const Interaction &it, const Scene &scene,
                                MemoryArena &arena, Sampler &sampler,
                                const std::vector<int> &nLightSamples,
                                bool handleMedia = false);

Spectrum uniformSampleOneLight(const Interaction &it, const Scene &scene,
                               MemoryArena &arena, Sampler &sampler,
                               bool handleMedia = false,
                               const Distribution1D *lightDistrib = nullptr);

Spectrum estimateDirect(const Interaction &it, const Point2f &uShading,
                        const Light &light, const Point2f &uLight,
                        const Scene &scene, Sampler &sampler,
                        MemoryArena &arena, bool handleMedia = false,
                        bool specular = false);

PALADIN_END

#endif /* integrator_hpp */
