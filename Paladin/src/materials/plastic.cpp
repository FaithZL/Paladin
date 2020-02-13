//
//  plastic.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/9/24.
//

#include "plastic.hpp"
#include "core/bxdf.hpp"
#include "core/spectrum.hpp"
#include "core/texture.hpp"
#include "bxdfs/lambert.hpp"
#include "materials/bxdfs/bsdf.hpp"

PALADIN_BEGIN

void PlasticMaterial::computeScatteringFunctions(
    SurfaceInteraction *si, MemoryArena &arena, TransportMode mode,
    bool allowMultipleLobes) const {
    
    if (_bumpMap) {
    	bump(_bumpMap, si);
    }
    si->bsdf = ARENA_ALLOC(arena, BSDF)(*si);
    Spectrum kd = _Kd->evaluate(*si).clamp();
    if (!kd.IsBlack()) {
        si->bsdf->add(ARENA_ALLOC(arena, LambertianReflection)(kd));
    }

    Spectrum ks = _Ks->evaluate(*si).clamp();
    if (!ks.IsBlack()) {
        Fresnel *fresnel = ARENA_ALLOC(arena, FresnelDielectric)(1.f, 1.5f);
        
        Float rough = _roughness->evaluate(*si);
        if (_remapRoughness) {
            rough = GGXDistribution::RoughnessToAlpha(rough);
        }
        MicrofacetDistribution *distrib =
            ARENA_ALLOC(arena, GGXDistribution)(rough, rough);
        BxDF *spec =
            ARENA_ALLOC(arena, MicrofacetReflection)(ks, distrib, fresnel);
        si->bsdf->add(spec);
    }
}

//"param" : {
//    "Kd" : {
//        "type" : "constant",
//        "param" : {
//            "colorType" : 0,
//            "color" : [0.1, 0.9, 0.5]
//        }
//    },
//    "Ks" : {
//        "type" : "constant",
//        "param" : {
//            "colorType" : 0,
//            "color" : [0.1, 0.9, 0.5]
//        }
//    },
//    "rough" : {
//        "type" : "constant",
//        "param" : 0.5
//    },
//    "bumpMap" : {
//        "type" : "constant",
//        "param" : 0.5
//    },
//    "remapRough" : false
//}
CObject_ptr createPlastic(const nloJson &param, const Arguments &lst) {
    nloJson _Kd = param.value("Kd", nloJson::object());
    auto Kd = shared_ptr<Texture<Spectrum>>(createSpectrumTexture(_Kd));
    
    nloJson _Ks = param.value("Ks", nloJson::object());
    auto Ks = shared_ptr<Texture<Spectrum>>(createSpectrumTexture(_Ks));
    
    nloJson _rough = param.value("rough", nloJson::object());
    auto rough = shared_ptr<Texture<Float>>(createFloatTexture(_rough));
    
    nloJson _bumpMap = param.value("bumpMap", nloJson::object());
    auto bumpMap = shared_ptr<Texture<Float>>(createFloatTexture(_bumpMap));
    
    bool remap = param.value("remapRough", false);
    
    auto ret = new PlasticMaterial(Kd, Ks, rough, bumpMap, remap);
    return ret;
}

REGISTER("plastic", createPlastic)

PALADIN_END
