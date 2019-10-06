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

PALADIN_END
