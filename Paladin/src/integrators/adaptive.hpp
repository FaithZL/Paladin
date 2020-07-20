//
//  adaptive.hpp
//  Paladin
//
//  Created by Zero on 2020/7/19.
//

#ifndef adaptive_hpp
#define adaptive_hpp

#include "core/integrator.hpp"

PALADIN_BEGIN

//http://luthuli.cs.uiuc.edu/~daf/courses/rendering/Papers-2/RTHWJ.article.pdf
//https://www.cs.umd.edu/~zwicker/publications/MultidimensionalAdaptiveSampling-SIG08.pdf
//http://ima.udg.edu/~rigau/publications/rigau03b.pdf
class AdaptiveIntegrator : public MonteCarloIntegrator {
    
public:
    
    AdaptiveIntegrator(std::shared_ptr<const Camera> camera,
                       std::shared_ptr<Sampler> sampler,
                       const AABB2i &pixelBound,
                       MonteCarloIntegrator * integrator,
                       Float maxErr)
    : MonteCarloIntegrator(camera, sampler, pixelBound) {
        _subIntegrator.reset(integrator);
    }
    
    virtual void render(const Scene &);
    
    virtual Spectrum Li(const RayDifferential &ray, const Scene &scene,
                        Sampler &sampler, MemoryArena &arena,
                        int depth = 0) const {
        return _subIntegrator->Li(ray, scene, sampler, arena, depth);
    }
    
private:
    unique_ptr<MonteCarloIntegrator> _subIntegrator;
    
    Float _maxError;
    Float _quantile;
    Float _pValue;
    Float _averageLuminance;
    Float _maxSpp;
};

PALADIN_END

#endif /* adaptive_hpp */
