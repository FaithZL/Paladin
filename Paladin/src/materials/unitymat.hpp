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
                  const std::shared_ptr<Texture<Spectrum>> &normalMap,
                  std::shared_ptr<Texture<Float>> bumpMap = nullptr)
    : Material(normalMap, bumpMap),
    _albedo(albedo),
    _metallic(metallic),
    _roughness(roughness),
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
    
    shared_ptr<ScaleTexture<Texture<Spectrum>, Spectrum>> _specular;
    shared_ptr<Texture<Spectrum>> _F0;
    
    bool _remapRoughness;
};

CObject_ptr createUnityMaterial(const nloJson &, const Arguments &lst);

PALADIN_END

#endif /* unitymat_hpp */
