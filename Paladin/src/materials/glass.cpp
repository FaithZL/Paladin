//
//  glass.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/9/24.
//

#include "glass.hpp"
#include "core/interaction.hpp"
#include "core/bxdf.hpp"
#include "core/texture.hpp"

PALADIN_BEGIN

void GlassMaterial::computeScatteringFunctions(SurfaceInteraction *si,
                                               MemoryArena &arena,
                                               TransportMode mode,
                                               bool allowMultipleLobes) const {
    if (_bumpMap) {
        bump(_bumpMap, si);
    }
    Float eta = _eta->evaluate(*si);
    Float urough = _uRoughness->evaluate(*si);
    Float vrough = _vRoughness->evaluate(*si);
    Spectrum R = _Kr->evaluate(*si).clamp();
    Spectrum T = _Kt->evaluate(*si).clamp();
    
    si->bsdf = ARENA_ALLOC(arena, BSDF)(*si,eta);
    
    if (R.IsBlack() && T.IsBlack()) {
        return;
    }
    
    bool isSpecular = urough == 0 && vrough == 0;

    Float etaMedium = 1.0f;
    
    if (isSpecular && allowMultipleLobes) {
        // 这里的介质折射率不应该固定为1.0f，应该根据介质  todo
        FresnelSpecular * fr = ARENA_ALLOC(arena, FresnelSpecular)(R, T, etaMedium, eta, mode);
        si->bsdf->add(fr);
    } else {
        if (_remapRoughness) {
            urough = TrowbridgeReitzDistribution::RoughnessToAlpha(urough);
            vrough = TrowbridgeReitzDistribution::RoughnessToAlpha(vrough);
        }
        
        MicrofacetDistribution * distrib = isSpecular ?
                                            nullptr :
                                            ARENA_ALLOC(arena, TrowbridgeReitzDistribution)(urough, vrough);
        if (!R.IsBlack()) {
            Fresnel *fresnel = ARENA_ALLOC(arena, FresnelDielectric)(etaMedium, eta);
            if (isSpecular) {
                BxDF * bxdf = ARENA_ALLOC(arena, SpecularReflection)(R, fresnel);
                si->bsdf->add(bxdf);
            } else {
                BxDF * bxdf = ARENA_ALLOC(arena, MicrofacetReflection)(R, distrib, fresnel);
                si->bsdf->add(bxdf);
            }
        }
        if (!T.IsBlack()) {
            if (isSpecular) {
                BxDF * bxdf = ARENA_ALLOC(arena, SpecularTransmission)(T, etaMedium, eta, mode);
                si->bsdf->add(bxdf);
            } else {
                BxDF * bxdf = ARENA_ALLOC(arena, MicrofacetTransmission)(T, distrib, etaMedium, eta, mode);
                si->bsdf->add(bxdf);
            }
        }
    }
}

//"param" : {
//    "Kr" : {
//        "type" : "constant",
//        "param" : {
//            "colorType" : 0,
//            "color" : [0.1, 0.9, 0.5]
//        }
//    },
//    "Kt" : {
//        "type" : "constant",
//        "param" : {
//            "colorType" : 0,
//            "color" : [0.1, 0.9, 0.5]
//        }
//    },
//    "uRough" : {
//        "type" : "constant",
//        "param" : 0.5
//    },
//    "vRough" : {
//        "type" : "constant",
//        "param" : 0.5
//    },
//    "eta" : {
//        "type" : "constant",
//        "param" : 0.5
//    },
//    "bumpMap" : {
//        "type" : "constant",
//        "param" : 0.5
//    },
//    "remapRough" : false
//}
CObject_ptr createGlass(const nloJson &param, const Arguments &lst) {
    
    nloJson _Kr = param.value("Kr", nloJson::object());
    auto Kr = shared_ptr<Texture<Spectrum>>(createSpectrumTexture(_Kr));
    
    nloJson _Kt = param.value("Kt", nloJson::object());
    auto Kt = shared_ptr<Texture<Spectrum>>(createSpectrumTexture(_Kt));
    
    nloJson _uRough = param.value("uRough", nloJson::object());
    auto uRough = shared_ptr<Texture<Float>>(createFloatTexture(_uRough));
    
    nloJson _vRough = param.value("vRough", nloJson::object());
    auto vRough = shared_ptr<Texture<Float>>(createFloatTexture(_vRough));
    
    nloJson _eta = param.value("eta", nloJson::object());
    auto eta = shared_ptr<Texture<Float>>(createFloatTexture(_eta));
    
    nloJson _bumpMap = param.value("bumpMap", nloJson::object());
    auto bumpMap = shared_ptr<Texture<Float>>(createFloatTexture(_bumpMap));
    
    bool remap = param.value("remapRough", false);
    
    auto ret = new GlassMaterial(Kr, Kt, uRough, vRough, eta, bumpMap, remap);
    return ret;
}

REGISTER("glass", createGlass)

PALADIN_END

