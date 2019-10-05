//
//  metal.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/9/24.
//

#ifndef metal_hpp
#define metal_hpp

#include "core/header.h"
#include "core/material.hpp"

PALADIN_BEGIN

class MetalMaterial : public Material {
public:
    MetalMaterial(const std::shared_ptr<Texture<Spectrum>> &eta,
                  const std::shared_ptr<Texture<Spectrum>> &k,
                  const std::shared_ptr<Texture<Float>> &rough,
                  const std::shared_ptr<Texture<Float>> &urough,
                  const std::shared_ptr<Texture<Float>> &vrough,
                  const std::shared_ptr<Texture<Float>> &bump,
                  bool remapRoughness)
    :_eta(eta),
    _k(k),
    _roughness(roughness),
    _uRoughness(uRoughness),
    _vRoughness(vRoughness),
    _bumpMap(bumpMap),
    _remapRoughness(remapRoughness) {
        
    }
    
    virtual void computeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena,
                                    TransportMode mode,
                                    bool allowMultipleLobes) const;
    
private:
    std::shared_ptr<Texture<Spectrum>> _eta, _k;
    std::shared_ptr<Texture<Float>> _roughness, _uRoughness, _vRoughness;
    std::shared_ptr<Texture<Float>> _bumpMap;
    bool _remapRoughness;
};

PALADIN_END

#endif /* metal_hpp */
