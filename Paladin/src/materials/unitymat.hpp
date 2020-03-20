//
//  unitymat.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/2/25.
//

#ifndef unitymat_hpp
#define unitymat_hpp

#include "core/material.hpp"
#include "textures/scale.hpp"

PALADIN_BEGIN

class UnityMaterial : public Material {
    
public:
    
    UnityMaterial(const shared_ptr<ScaleTexture<Spectrum, Spectrum>>& albedo,
                  const std::shared_ptr<Texture<Float>>& metallic,
                  const std::shared_ptr<Texture<Float>>& roughness,
                  bool remapRoughness,
                  const shared_ptr<Texture<Spectrum>> &F0,
                  const std::shared_ptr<Texture<Spectrum>> &normalMap,
                  const shared_ptr<Texture<Float>> &bumpMap = nullptr,
                  Float scale = -1)
    : Material(normalMap, bumpMap, scale),
    _albedo(albedo),
    _metallic(metallic),
    _roughness(roughness),
    _F0(F0),
    _remapRoughness(remapRoughness) {
        
    }
    
    virtual nloJson toJson() const override {
        return nloJson();
    }
    
    virtual void computeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena,
                                    TransportMode mode,
                                    bool allowMultipleLobes) const override;
private:
    
    shared_ptr<ScaleTexture<Spectrum, Spectrum>> _albedo;
    std::shared_ptr<Texture<Float>> _metallic;
    std::shared_ptr<Texture<Float>> _roughness;
    std::shared_ptr<Texture<Spectrum>> _opacity;
    shared_ptr<Texture<Spectrum>> _F0;
    bool _remapRoughness;
};

CObject_ptr createUnityMaterial(const nloJson &, const Arguments &lst);

PALADIN_END

#endif /* unitymat_hpp */
