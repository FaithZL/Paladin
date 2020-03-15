//
//  mirror.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/9/24.
//

#ifndef mirror_hpp
#define mirror_hpp

#include "core/material.hpp"

PALADIN_BEGIN

class MirrorMaterial : public Material {
    
public:
    MirrorMaterial(const std::shared_ptr<Texture<Spectrum>> &r,
                   const std::shared_ptr<Texture<Spectrum>> &normalMap,
                   const std::shared_ptr<Texture<Float>> &bump)
    :Material(normalMap, bump),
    _Kr(r) {
        
    }
    
    virtual nloJson toJson() const override {
        return nloJson();
    }
    
    virtual void computeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena,
                                            TransportMode mode,
                                            bool allowMultipleLobes) const override;
    
private:
    std::shared_ptr<Texture<Spectrum>> _Kr;
};

CObject_ptr createMirror(const nloJson &param, const Arguments &lst);

PALADIN_END

#endif /* mirror_hpp */
