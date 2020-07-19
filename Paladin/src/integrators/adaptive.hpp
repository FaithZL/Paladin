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

class AdaptiveIntegrator : public Integrator {
    
public:
    
    AdaptiveIntegrator(Integrator * integrator) {
        _subIntegrator.reset(integrator);
    }
    
    virtual void render(const Scene &);
    
    virtual Spectrum Li(const RayDifferential &ray, const Scene &scene,
                        Sampler &sampler, MemoryArena &arena,
                        int depth = 0) const {
        
    }
    
private:
    unique_ptr<Integrator> _subIntegrator;
    
    Float _maxError;
    Float _quantile;
    Float _pValue;
    Float _averageLuminance;
    
};

PALADIN_END

#endif /* adaptive_hpp */
