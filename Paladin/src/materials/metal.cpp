//
//  metal.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/9/24.
//

#include "metal.hpp"
#include "core/bxdf.hpp"
#include "core/texture.hpp"

PALADIN_BEGIN

void MetalMaterial::computeScatteringFunctions(SurfaceInteraction *si,
                                               MemoryArena &arena,
                                               TransportMode mode,
                                               bool allowMultipleLobes) const {
    if (_bumpMap) {
        bump(_bumpMap, si);
    }
    si->bsdf = ARENA_ALLOC(arena, BSDF)(*si);
    
    Float uRough = _uRoughness ? _uRoughness->evaluate(*si) : _roughness->evaluate(*si);
    Float vRough = _vRoughness ? _vRoughness->evaluate(*si) : _roughness->evaluate(*si);
    if (_remapRoughness) {
        uRough = TrowbridgeReitzDistribution::RoughnessToAlpha(uRough);
        vRough = TrowbridgeReitzDistribution::RoughnessToAlpha(vRough);
    }
    Fresnel *frMf = ARENA_ALLOC(arena, FresnelConductor)(1., _eta->evaluate(*si),
                                                         _k->evaluate(*si));
    MicrofacetDistribution *distrib =
    ARENA_ALLOC(arena, TrowbridgeReitzDistribution)(uRough, vRough);
    si->bsdf->add(ARENA_ALLOC(arena, MicrofacetReflection)(1., distrib, frMf));
}

//"param" : {
//    "eta" : {
//        "type" : "constant",
//        "param" : {
//            "colorType" : 0,
//            "color" : [0.1, 0.9, 0.5]
//        }
//    },
//    "k" : {
//        "param" : {
//            "colorType" : 0,
//            "color" : [0.1, 0.9, 0.5]
//        }
//    },
//    "rough" : {
//        "type" : "constant",
//        "param" : 0.5
//    },
//    "uRough" : {
//        "type" : "constant",
//        "param" : 0.5
//    },
//    "vRough" : {
//        "type" : "constant",
//        "param" : 0.5
//    },
//    "bumpMap" : {
//        "type" : "constant",
//        "param" : 0.5
//    },
//    "remapRough" : false
//}
CObject_ptr createMetal(const nloJson &param, const Arguments &lst) {
    nloJson _eta = param.value("eta", nloJson::object());
    auto eta = shared_ptr<Texture<Spectrum>>(createSpectrumTexture(_eta));
    
    nloJson _k = param.value("k", nloJson::object());
    auto k = shared_ptr<Texture<Spectrum>>(createSpectrumTexture(_k));
    
    nloJson _rough = param.value("rough", nloJson::object());
    auto rough = shared_ptr<Texture<Float>>(createFloatTexture(_rough));
    
    nloJson _uRough = param.value("uRough", nloJson::object());
    auto uRough = shared_ptr<Texture<Float>>(createFloatTexture(_uRough));
    
    nloJson _vRough = param.value("vRough", nloJson::object());
    auto vRough = shared_ptr<Texture<Float>>(createFloatTexture(_vRough));
    
    nloJson _bumpMap = param.value("bumpMap", nloJson::object());
    auto bumpMap = shared_ptr<Texture<Float>>(createFloatTexture(_bumpMap));
    
    bool remap = param.value("remapRough", false);
    
    auto ret = new MetalMaterial(eta, k, rough, uRough, vRough, bumpMap, remap);
    
    return ret;
}

REGISTER("metal", createMetal)

PALADIN_END
