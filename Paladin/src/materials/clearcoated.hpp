//
//  clearcoated.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/1/17.
//

#ifndef coated_hpp
#define coated_hpp

#include "core/material.hpp"

PALADIN_BEGIN

// 有透明涂层的材质
class ClearCoatedMaterial : public Material {
    
public:
    
    ClearCoatedMaterial(const std::shared_ptr<Texture<Spectrum>> &Ks,
                        const std::shared_ptr<Texture<Spectrum>> &Kd,
                        const std::shared_ptr<Texture<Float>> &urough,
                        const std::shared_ptr<Texture<Float>> &vrough,
                        bool remapRough,
                        const std::shared_ptr<Texture<Float>> &bumpMap)
    : _Ks(Ks),
    _Kd(Kd),
    _uRoughness(urough),
    _vRoughness(vrough),
    _remapRoughness(remapRough),
    _bumpMap(bumpMap) {
        
    }
    
    virtual nloJson toJson() const override {
        return nloJson();
    }
    
    virtual void computeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena,
                                    TransportMode mode,
                                    bool allowMultipleLobes) const override;
    
private:
    std::shared_ptr<Texture<Spectrum>> _Ks, _Kd;
    std::shared_ptr<Texture<Float>> _uRoughness, _vRoughness;
    bool _remapRoughness;
    std::shared_ptr<Texture<Float>> _bumpMap;
};

CObject_ptr createClearCoated(const nloJson &param, const Arguments &lst);

REGISTER("clearcoated", createClearCoated);

PALADIN_END

#endif /* clearcoated_hpp */
