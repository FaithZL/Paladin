//
//  mirror.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/9/24.
//

#include "mirror.hpp"
#include "core/interaction.hpp"
#include "core/bxdf.hpp"
#include "core/texture.hpp"

PALADIN_BEGIN

void MirrorMaterial::computeScatteringFunctions(SurfaceInteraction *si,
                                                MemoryArena &arena,
                                                TransportMode mode,
                                                bool allowMultipleLobes) const {
    if (_bumpMap) {
        bump(_bumpMap, si);
    }
    si->bsdf = ARENA_ALLOC(arena, BSDF)(*si);
    Spectrum R = _Kr->evaluate(*si).clamp();
    if (!R.IsBlack()) {
        FresnelNoOp * fresnel = ARENA_ALLOC(arena, FresnelNoOp)();
        SpecularReflection * sr = ARENA_ALLOC(arena, SpecularReflection)(R, fresnel);
        si->bsdf->add(sr);
    }
    
}

//"Kr" : {
//    "type" : "constant",
//    "param" : {
//        "colorType" : 0,
//        "color" : [0.1, 0.9, 0.5]
//    }
//}
CObject_ptr createMirror(const nloJson &param, const Arguments &lst) {
    nloJson _Kr = param.value("Kr", nloJson::object());
    auto Kr = shared_ptr<Texture<Spectrum>>(createSpectrumTexture(_Kr));
    
    nloJson _bumpMap = param.value("bumpMap", nloJson());
    auto bumpMap = shared_ptr<Texture<Float>>(createFloatTexture(_bumpMap));
    
    auto ret = new MirrorMaterial(Kr, bumpMap);
    return ret;
}

REGISTER("mirror", createMirror)

PALADIN_END
