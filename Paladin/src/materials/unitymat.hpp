//
//  unitymat.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/2/25.
//

#ifndef unitymat_hpp
#define unitymat_hpp

#include "core/material.hpp"

PALADIN_BEGIN

class UnityMaterial : public Material {
    
public:
    
    UnityMaterial(const std::shared_ptr<Texture<Spectrum>>& albedo,
                  const std::shared_ptr<Texture<Float>>& metallic,
                  const std::shared_ptr<Texture<Float>>& roughness)
    :_albedo(albedo),
    _metallic(metallic),
    _roughness(roughness){
        
    }
    
    virtual nloJson toJson() const override {
        return nloJson();
    }
    
    virtual void computeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena,
                                    TransportMode mode,
                                    bool allowMultipleLobes) const override;
private:
    
    std::shared_ptr<Texture<Spectrum>> _albedo;
    std::shared_ptr<Texture<Float>> _metallic;
    std::shared_ptr<Texture<Float>> _roughness;
};

CObject_ptr createUnityMaterial(const nloJson &, const Arguments &lst);

PALADIN_END

#endif /* unitymat_hpp */
