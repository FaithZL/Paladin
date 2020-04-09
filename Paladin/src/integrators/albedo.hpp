//
//  albedo.hpp
//  Paladin
//
//  Created by Zero on 2020/4/9.
//

#ifndef albedo_hpp
#define albedo_hpp

#include "core/integrator.hpp"

PALADIN_BEGIN

class AlbedoIntegrator : public MonteCarloIntegrator {
    
public:
    AlbedoIntegrator(const std::shared_ptr<const Camera>& camera,
                     const std::shared_ptr<Sampler>& sampler,
                     const AABB2i& pixelBound)
    : MonteCarloIntegrator(camera,sampler,pixelBound) {
        
    }
    
    virtual nloJson toJson() const override {
        return nloJson();
    }
    
    virtual Spectrum Li(const RayDifferential &ray, const Scene &scene,
                    Sampler &sampler, MemoryArena &arena,
                        int depth = 0) const override {
        SurfaceInteraction ref;//和表面的交互点
        RGBSpectrum ret(0.0f);
        
        if (scene.intersect(ray, &ref)) {
            
        }
        
        return ret;
    }
    
};

PALADIN_END

#endif /* albedo_hpp */
