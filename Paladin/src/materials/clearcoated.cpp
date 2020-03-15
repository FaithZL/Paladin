//
//  clearcoated.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/1/17.
//

#include "clearcoated.hpp"
#include "core/texture.hpp"
#include "bxdfs/bsdf.hpp"
#include "bxdfs/microfacet/reflection.hpp"
#include "bxdfs/microfacet/fresnelblend.hpp"

PALADIN_BEGIN

void ClearCoatedMaterial::computeScatteringFunctions(SurfaceInteraction *si,
                                                     MemoryArena &arena,
                                                     TransportMode mode,
                                                     bool allowMultipleLobes) const {
    processNormal(si);
    
    si->bsdf = ARENA_ALLOC(arena, BSDF)(*si);
    Float urough = _uRoughness->evaluate(*si);
    Float vrough = _vRoughness->evaluate(*si);
    Spectrum Rd = _Kd->evaluate(*si).clamp();
    Spectrum Rs = _Ks->evaluate(*si).clamp();
    
    if (_remapRoughness) {
        urough = BeckmannDistribution::RoughnessToAlpha(urough);
        vrough = BeckmannDistribution::RoughnessToAlpha(vrough);
    }
    auto distrib = ARENA_ALLOC(arena, BeckmannDistribution)(urough, vrough);
    BxDF * bxdf = ARENA_ALLOC(arena, FresnelBlend)(Rd, Rs, distrib);
    si->bsdf->add(bxdf);
    
}

//"param" : {
//    "Kd" : [
//        0.629999995231628,
//        0.0649999976158142,
//        0.0500000007450581
//    ],
//    "uRough" : 0.5,
//    "vRough" : 0.5,
//    "Ks" : [1,1,1],
//    "remapRough" : false
//}
CObject_ptr createClearCoated(const nloJson &param, const Arguments &lst) {
    nloJson _Kd = param.value("Kd", nloJson::array({1.f, 1.f, 1.f}));
    auto Kd = shared_ptr<Texture<Spectrum>>(createSpectrumTexture(_Kd));
    
    nloJson _Ks = param.value("Kd", nloJson::array({1.f, 1.f, 1.f}));
    auto Ks = shared_ptr<Texture<Spectrum>>(createSpectrumTexture(_Ks));
    
    nloJson _uRough = param.value("uRough", nloJson(0.f));
    auto uRough = shared_ptr<Texture<Float>>(createFloatTexture(_uRough));
    
    nloJson _vRough = param.value("vRough", nloJson(0.f));
    auto vRough = shared_ptr<Texture<Float>>(createFloatTexture(_vRough));
    
    nloJson _bumpMap = param.value("bumpMap", nloJson());
    auto bumpMap = shared_ptr<Texture<Float>>(createFloatTexture(_bumpMap));
    
    nloJson _normalMap = param.value("normalMap", nloJson());
    auto normalMap = shared_ptr<Texture<Spectrum>>(createSpectrumTexture(_normalMap));
    
    bool remapRough = param.value("remapRough", false);
    
    return new ClearCoatedMaterial(Ks, Kd, uRough, vRough,
                                   remapRough, normalMap, bumpMap);
}

PALADIN_END
