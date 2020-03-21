//
//  glass.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/9/24.
//

#ifndef glass_hpp
#define glass_hpp

#include "core/material.hpp"

PALADIN_BEGIN

class GlassMaterial : public Material {
    
public:
    GlassMaterial(const std::shared_ptr<Texture<Spectrum>> &Kr,
                  const std::shared_ptr<Texture<Spectrum>> &Kt,
                  const std::shared_ptr<Texture<Float>> &uRoughness,
                  const std::shared_ptr<Texture<Float>> &vRoughness,
                  const std::shared_ptr<Texture<Float>> &eta,
                  const std::shared_ptr<Texture<Spectrum>> &normalMap,
                  const std::shared_ptr<Texture<Float>> &bumpMap,
                  bool remapRoughness,
                  bool thin = false)
    : Material(normalMap, bumpMap),
    _Kr(Kr),
    _Kt(Kt),
    _uRoughness(uRoughness),
    _vRoughness(vRoughness),
    _eta(eta),
    _remapRoughness(remapRoughness),
    _thin(thin) {
        
    }
    
    virtual nloJson toJson() const override {
        return nloJson();
    }
    
    virtual void computeScatteringFunctions(SurfaceInteraction *si,
                                            MemoryArena &arena,
                                            TransportMode mode,
                                            bool allowMultipleLobes) const override;
    
private:
    std::shared_ptr<Texture<Spectrum>> _Kr, _Kt;
    std::shared_ptr<Texture<Float>> _uRoughness, _vRoughness;
    std::shared_ptr<Texture<Float>> _eta;
    bool _remapRoughness;
    bool _thin;
};

CObject_ptr createGlass(const nloJson &param, const Arguments &lst);

PALADIN_END

#endif /* glass_hpp */
