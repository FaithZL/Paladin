//
//  unite.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/1/8.
//

#include "hyper.hpp"
#include "core/bxdf.hpp"
#include "core/spectrum.hpp"
#include "core/texture.hpp"
#include "bxdfs/lambert.hpp"
#include "bxdfs/specular.hpp"
#include "materials/bxdfs/bsdf.hpp"

PALADIN_BEGIN

void HyperMaterial::computeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena, TransportMode mode, bool allowMultipleLobes) const {
    
    if (_bumpMap) {
        bump(_bumpMap, si);
    }
    
    Float eta = _eta->evaluate(*si);
    // opacity不透明度
    Spectrum opacity = _opacity->evaluate(*si).clamp();
    // t为透明度
    Spectrum t = (Spectrum(1.f) - opacity).clamp();
    if (!t.IsBlack()) {
        si->bsdf = ARENA_ALLOC(arena, BSDF)(*si, 1.f);
        // thin glass approximation
        // unite材质中的透明度参数用于模拟很薄的玻璃，所以两侧的折射率均为1
        auto *tr = ARENA_ALLOC(arena, SpecularTransmission)(t, 1.f, 1.f, mode);
        si->bsdf->add(tr);
    } else {
        si->bsdf = ARENA_ALLOC(arena, BSDF)(*si, eta);
    }
    
    Spectrum Kd = opacity * _Kd->evaluate(*si).clamp();
    if (!Kd.IsBlack()) {
        auto diff = ARENA_ALLOC(arena, LambertianReflection)(Kd);
        si->bsdf->add(diff);
    }
    
    Spectrum Ks = opacity * _Ks->evaluate(*si);
    if (!Ks.IsBlack()) {
        // ks用于计算反射
        auto * fresnel = ARENA_ALLOC(arena, FresnelDielectric)(1.f, eta);
        Float rough_u, rough_v;
        if (_roughness_u) {
            rough_u = _roughness_u->evaluate(*si);
        } else {
            rough_u = _roughness->evaluate(*si);
        }
        if (_roughness_v) {
            rough_v = _roughness_v->evaluate(*si);
        } else {
            rough_v = _roughness->evaluate(*si);
        }
        if (_remapRoughness) {
            rough_u = GGXDistribution::RoughnessToAlpha(rough_u);
            rough_v = GGXDistribution::RoughnessToAlpha(rough_v);
        }
        MicrofacetDistribution * d = ARENA_ALLOC(arena, GGXDistribution)(rough_u,rough_v);
        
        auto mSpec = ARENA_ALLOC(arena, MicrofacetReflection)(Ks, d, fresnel);
        si->bsdf->add(mSpec);
    }
    
    if (_Kr) {
        Spectrum Kr = opacity * _Kr->evaluate(*si);
        if (!Kr.IsBlack()) {
            auto fresnel = ARENA_ALLOC(arena, FresnelDielectric)(1.f, eta);
            auto specR = ARENA_ALLOC(arena, SpecularReflection)(Kr, fresnel);
            si->bsdf->add(specR);
        }
    }
    if (_Kt) {
        Spectrum Kt = t * _Kt->evaluate(*si);
        if (!Kt.IsBlack()) {
            auto specT = ARENA_ALLOC(arena, SpecularTransmission)(Kt, 1.f, eta, mode);
            si->bsdf->add(specT);
        }
    }
}


shared_ptr<HyperMaterial> HyperMaterial::create(const std::shared_ptr<Texture<Spectrum>> &Kd,
                                                const std::shared_ptr<Texture<Spectrum>> &Ks,
                                                const std::shared_ptr<Texture<Spectrum>> &Kr,
                                                const std::shared_ptr<Texture<Spectrum>> &Kt,
                                                const std::shared_ptr<Texture<Float>> &roughness,
                                                const std::shared_ptr<Texture<Float>> &roughnessu,
                                                const std::shared_ptr<Texture<Float>> &roughnessv,
                                                const std::shared_ptr<Texture<Spectrum>> &opacity,
                                                const std::shared_ptr<Texture<Float>> &eta,
                                                const std::shared_ptr<Texture<Float>> &bumpMap,
                                                bool remapRoughness) {
    return make_shared<HyperMaterial>(Kd, Ks, Kr, Kt, roughness, roughnessu, roughnessv, opacity, eta, bumpMap, remapRoughness);
}

PALADIN_END
