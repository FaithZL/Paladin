//
//  mix.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/2.
//

#ifndef material_mix_hpp
#define material_mix_hpp

#include "core/header.h"
#include "core/material.hpp"

PALADIN_BEGIN

class MixMaterial : public Material {
    
public:
    
    MixMaterial(const std::shared_ptr<Material> &m1,
                const std::shared_ptr<Material> &m2,
                const std::shared_ptr<Texture<Spectrum>> &scale)
    : _m1(m1), _m2(m2), _scale(scale) {
        
    }
    
    virtual void computeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena,
                                            TransportMode mode,
                                            bool allowMultipleLobes) const;
    
private:
    std::shared_ptr<Material> _m1, _m2;
    std::shared_ptr<Texture<Spectrum>> _scale;
};

PALADIN_END

#endif /* material_mix_hpp */
