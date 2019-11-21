//
//  matte.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "matte.hpp"
#include "core/bxdf.hpp"
#include "core/interaction.hpp"
#include "core/texture.hpp"

PALADIN_BEGIN

void MatteMaterial::computeScatteringFunctions(SurfaceInteraction *si,
                                               MemoryArena &arena,
                                               TransportMode mode,
                                               bool allowMultipleLobes) const {
	if (_bumpMap) {
		bump(_bumpMap, si);
	}

	si->bsdf = ARENA_ALLOC(arena, BSDF)(*si);
	Spectrum r = _Kd->evaluate(*si).clamp();
	Float sig = clamp(_sigma->evaluate(*si), 0, 90);
	if (!r.IsBlack()) {
		if (sig == 0) {
			// 如果粗糙度为零，朗博反射
			si->bsdf->add(ARENA_ALLOC(arena, LambertianReflection)(r));
		} else {
			si->bsdf->add(ARENA_ALLOC(arena, OrenNayar)(r, sig));
		}
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
//    "sigma" : {
//        "type" : "constant",
//        "param" : 0
//    },
//    "bumpMap" : {
//        "type" : "constant",
//        "param" : 0
//    }
//}
CObject_ptr createMatte(const nloJson &param, const Arguments& lst) {
    nloJson _Kd = param.value("Kd", nloJson::object());
    Texture<Spectrum> * Kd = createTextureSpectrum(_Kd);
    nloJson _sigma = param.value("sigma", nloJson::object());
    nloJson _bumpMap = param.value("bumpMap", nloJson::object());
    
    
}

REGISTER("matte", createMatte)

PALADIN_END
