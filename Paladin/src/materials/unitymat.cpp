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
#include "bxdfs/microfacet/reflection.hpp"

PALADIN_BEGIN

void UnityMaterial::computeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena,
                                          TransportMode mode, bool allowMultipleLobes) const {
    si->bsdf = ARENA_ALLOC(arena, BSDF)(*si);
    
    Float metallic = _metallic->evaluate(*si);
    Float alpha = GGXDistribution::RoughnessToAlpha(_roughness->evaluate(*si));
    auto albedo = _albedo->evaluate(*si);
    
    // 如果完全光滑
    if (alpha == 0) {
        auto diffuse = ARENA_ALLOC(arena,LambertianReflection(albedo));
        auto scaled_diffuse = ARENA_ALLOC(arena,ScaledBxDF(diffuse, 1 - metallic));
        si->bsdf->add(scaled_diffuse);
    } else {
        auto diffuse= ARENA_ALLOC(arena,OrenNayar(albedo, alpha));
        auto scaled_diffuse=ARENA_ALLOC(arena,ScaledBxDF(diffuse, 1 - metallic));
        si->bsdf->add(scaled_diffuse);
    }
    
    GGXDistribution * ggx = ARENA_ALLOC(arena, GGXDistribution)(alpha, alpha);
    Fresnel * fresnel = ARENA_ALLOC(arena, FresnelSchlick)(albedo);
    auto specular = ARENA_ALLOC(arena, MicrofacetReflection)(1.0, ggx, fresnel);
    auto scaled_specular = ARENA_ALLOC(arena,ScaledBxDF(specular, metallic));
    si->bsdf->add(scaled_specular);
}

//"param" : {
//    "albedo" : [0.725, 0.71, 0.68],
//    "roughness" : 0.2,
//    "metallic" : 0.8
//}
CObject_ptr createUnityMaterial(const nloJson &param, const Arguments &lst) {
    nloJson _albedo = param.value("albedo", nloJson::array({1.f, 1.f, 1.f}));
    auto albedo = shared_ptr<Texture<Spectrum>>(createSpectrumTexture(_albedo));
    nloJson _roughness = param.value("roughness", nloJson::object());
    auto roughness = shared_ptr<Texture<Float>>(createFloatTexture(_roughness));
    nloJson _metallic = param.value("metallic", nloJson::object());
    auto metallic = shared_ptr<Texture<Float>>(createFloatTexture(_metallic));
    return new UnityMaterial(albedo, metallic, roughness);
}

REGISTER("unity", createUnityMaterial);

PALADIN_END
