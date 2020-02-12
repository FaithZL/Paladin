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
#include "textures/constant.hpp"
#include "bxdfs/lambert.hpp"

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
	Float sig = _sigma ? clamp(_sigma->evaluate(*si), 0, 90) : 0;
	if (!r.IsBlack()) {
		if (sig == 0) {
			// 如果粗糙度为零，朗博反射
			si->bsdf->add(ARENA_ALLOC(arena, LambertianReflection)(r));
		} else {
			si->bsdf->add(ARENA_ALLOC(arena, OrenNayar)(r, sig));
		}
	}
}

shared_ptr<MatteMaterial> createLightMat() {
    ConstantTexture<Spectrum> * Kd = new ConstantTexture<Spectrum>(Spectrum(0.f));
    return make_shared<MatteMaterial>(shared_ptr<Texture<Spectrum>>(Kd), nullptr, nullptr);
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
    nloJson _Kd = param.value("Kd", nloJson::array({1.f, 1.f, 1.f}));
    auto Kd = shared_ptr<Texture<Spectrum>>(createSpectrumTexture(_Kd));
    nloJson _sigma = param.value("sigma", nloJson::object());
    auto sigma = shared_ptr<Texture<Float>>(createFloatTexture(_sigma));
    nloJson _bumpMap = param.value("bumpMap", nloJson::object());
    auto bumpMap = shared_ptr<Texture<Float>>(createFloatTexture(_bumpMap));
    auto ret = new MatteMaterial(Kd, sigma, bumpMap);
    return ret;
}

REGISTER("matte", createMatte)

PALADIN_END
