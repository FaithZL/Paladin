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

PALADIN_BEGIN

void UnityMaterial::computeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena,
                                          TransportMode mode, bool allowMultipleLobes) const {
    si->bsdf = ARENA_ALLOC(arena, BSDF)(*si);
    //1.计算金属度
    auto metallic_scale = _metallic->evaluate(*si);
}

PALADIN_END
