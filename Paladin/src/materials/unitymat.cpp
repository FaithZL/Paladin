//
//  unitymat.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/2/25.
//

#include "unitymat.hpp"
#include "core/interaction.hpp"
#include "bxdfs/bsdf.hpp"
#include "core/texture.hpp"
#include "bxdfs/microfacet/distribute.hpp"
#include "bxdfs/lambert.hpp"
#include "bxdfs/specular.hpp"
#include "bxdfs/microfacet/reflection.hpp"

PALADIN_BEGIN

void UnityMaterial::computeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena,
                                          TransportMode mode, bool allowMultipleLobes) const {
    
    processNormal(si);
    si->bsdf = ARENA_ALLOC(arena, BSDF)(*si);
    
    Float metallic = _metallic->evaluate(*si);
    Float alpha = _roughness->evaluate(*si);
    auto albedo = _albedo->evaluate(*si);
    Fresnel * fresnel = ARENA_ALLOC(arena, FresnelSchlick)(albedo);
    
    BxDF * diffuse = nullptr;
    BxDF * spec = nullptr;

    // 如果完全光滑
    if (alpha != 0) {
        alpha = correctRoughness(alpha);
        diffuse= ARENA_ALLOC(arena, OrenNayar(albedo, alpha * 90));
        GGXDistribution * ggx = ARENA_ALLOC(arena, GGXDistribution)(alpha, alpha);
        spec = ARENA_ALLOC(arena, MicrofacetReflection)(1.0, ggx, fresnel);
    } else {
        diffuse = ARENA_ALLOC(arena, LambertianReflection(albedo));
        spec = ARENA_ALLOC(arena, SpecularReflection)(1.0, fresnel);
    }
    
    auto scaled_diffuse = ARENA_ALLOC(arena, ScaledBxDF(diffuse, 1 - metallic));
    si->bsdf->add(scaled_diffuse);
    
    auto scaled_specular = ARENA_ALLOC(arena, ScaledBxDF(spec, metallic));
    si->bsdf->add(scaled_specular);
}

//"param" : {
//    "albedo" :  [
//        {
//            "type" : "constant",
//            "subtype" : "spectrum",
//            "param" : {
//                "colorType" : 0,
//                "color" : [0.1, 0.9, 0.5]
//            }
//        },
//        {
//            "type" : "constant",
//            "subtype" : "spectrum",
//            "param" : {
//                "colorType" : 0,
//                "color" : [0.1, 0.9, 0.5]
//            }
//        }
//    ],
//    "uvOffset" : [1,1,0,0],
//    "roughness" : 0.2,
//    "metallic" : 0.8
//}
CObject_ptr createUnityMaterial(const nloJson &param, const Arguments &lst) {
    nloJson _albedo = param.value("albedo", nloJson::array({1.f, 1.f, 1.f}));
    auto tmp = dynamic_cast<ScaleTexture<Spectrum, Spectrum>*>(createTexture(_albedo));
    auto albedo = shared_ptr<ScaleTexture<Spectrum, Spectrum>>(tmp);
    
    nloJson _roughness = param.value("roughness", nloJson::object());
    auto roughness = shared_ptr<Texture<Float>>(createFloatTexture(_roughness));
    
    nloJson _metallic = param.value("metallic", nloJson::object());
    auto metallic = shared_ptr<Texture<Float>>(createFloatTexture(_metallic));
    
    bool remapRoughness = param.value("remapRough", false);
    
    nloJson _F0 = param.value("F0", nloJson::array({1,1,1}));
    auto F0 = shared_ptr<Texture<Spectrum>>(createSpectrumTexture(_F0));
    
    nloJson _normalMap = param.value("normalMap", nloJson());
    auto normalMap = shared_ptr<Texture<Spectrum>>(createSpectrumTexture(_normalMap));
    
    nloJson _bumpMap = param.value("bumpMap", nloJson());
    auto bumpMap = shared_ptr<Texture<Float>>(createFloatTexture(_bumpMap));
    
    Float scale = param.value("scale", -1);
    
    return new UnityMaterial(albedo, metallic, roughness,
                             remapRoughness, F0, normalMap,
                             bumpMap, scale);
}

REGISTER("unity", createUnityMaterial);

PALADIN_END
