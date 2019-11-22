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
    _roughness(rough),
    _uRoughness(urough),
    _vRoughness(vrough),
    _bumpMap(bump),
    _remapRoughness(remapRoughness) {
        
    }
    
    virtual nloJson toJson() const override {
        return nloJson();
    }
    
    virtual void computeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena,
                                    TransportMode mode,
                                    bool allowMultipleLobes) const override;
    
private:
    // eta折射率，k吸收系数，详见bxdf.hpp文件
    std::shared_ptr<Texture<Spectrum>> _eta, _k;
    std::shared_ptr<Texture<Float>> _roughness, _uRoughness, _vRoughness;
    std::shared_ptr<Texture<Float>> _bumpMap;
    bool _remapRoughness;
};

CObject_ptr createMetal(const nloJson &param, const Arguments &lst);

PALADIN_END

#endif /* metal_hpp */
