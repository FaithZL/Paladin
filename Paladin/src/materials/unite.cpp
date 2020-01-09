//
//  unite.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/1/8.
//

#include "unite.hpp"
#include "core/bxdf.hpp"
#include "core/spectrum.hpp"
#include "core/texture.hpp"

PALADIN_BEGIN

void UniteMaterial::computeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena, TransportMode mode, bool allowMultipleLobes) const {
    
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
        BxDF *tr = ARENA_ALLOC(arena, SpecularTransmission)(t, 1.f, 1.f, mode);
        si->bsdf->add(tr);
    } else {
        si->bsdf = ARENA_ALLOC(arena, BSDF)(*si, eta);
    }
    
    Spectrum ks = opacity * _Ks->evaluate(*si).clamp();
}

PALADIN_END
