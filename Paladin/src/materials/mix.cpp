//
//  mix.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/2.
//

#include "mix.hpp"
#include "core/bxdf.hpp"
#include "core/texture.hpp"
PALADIN_BEGIN

void MixMaterial::computeScatteringFunctions(SurfaceInteraction *si,
                                             MemoryArena &arena,
                                             TransportMode mode,
                                             bool allowMultipleLobes) const {
    Spectrum s1 = _scale->evaluate(*si).Clamp();
    Spectrum s2 = (Spectrum(1.f) - s1).Clamp();
    SurfaceInteraction si2 = *si;
    _m1->computeScatteringFunctions(si, arena, mode, allowMultipleLobes);
    _m2->computeScatteringFunctions(&si2, arena, mode, allowMultipleLobes);
    
    int n1 = si->bsdf->numComponents();
    int n2 = si2.bsdf->numComponents();
    for (int i = 0; i < n1; ++i) {
        // 注意，这里是需要对bxdf数组，直接赋值，覆盖之前的bxdf
        si->bsdf->bxdfs[i] = ARENA_ALLOC(arena, ScaledBxDF)(si->bsdf->bxdfs[i], s1);
    }
    for (int i = 0; i < n2; ++i) {
        si->bsdf->add(ARENA_ALLOC(arena, ScaledBxDF)(si2.bsdf->bxdfs[i], s2));
    }
}

PALADIN_END
