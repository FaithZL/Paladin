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
#include "materials/bxdfs/bsdf.hpp"
#include "bxdfs/microfacet/reflection.hpp"
#include "tools/parallel.hpp"

PALADIN_BEGIN

void MatteMaterial::computeScatteringFunctions(SurfaceInteraction *si,
                                               MemoryArena &arena,
                                               TransportMode mode,
                                               bool allowMultipleLobes) const {
    
    processNormal(si);
    
    
    Spectrum r = _Kd->evaluate(*si).clamp();
    Float sig = _sigma ? clamp(_sigma->evaluate(*si), 0, 90) : 0;

    si->bsdf = ARENA_ALLOC(arena, BSDF)(*si);
    if (!r.IsBlack()) {
        if (sig == 0) {
            // 如果粗糙度为零，朗博反射
            si->bsdf->add(ARENA_ALLOC(arena, LambertianReflection)(r));
        } else {
            si->bsdf->add(ARENA_ALLOC(arena, OrenNayar)(r, sig));
        }
    }
    
//    if (!r.IsBlack()) {
//        int idx = getCurThreadIndex();
//        auto bsdf = _bsdfs[idx];
//        bsdf->updateGeometry(*si);
//        bsdf->clearBxDFs();
//        if (sig == 0) {
//            auto lambert = (LambertianReflection *)bsdf->getBxDF(0);
//            lambert->setReflection(r);
//            bsdf->add(lambert);
//        } else {
//            auto lambert = (OrenNayar *)bsdf->getBxDF(1);
//            lambert->setReflection(r);
//            bsdf->add(lambert);
//        }
//        si->bsdf = bsdf.get();
//    }
    
	
}

void MatteMaterial::initBSDF(BSDF *bsdf) {
    SurfaceInteraction si;
    auto a = _Kd->evaluate(si);
    auto lambert = make_shared<LambertianReflection>(a);
    bsdf->addBxDF(lambert);
    auto on = make_shared<OrenNayar>(a, 0);
    bsdf->addBxDF(on);
}

void MatteMaterial::updateScatteringFunctions(SurfaceInteraction *si,
                                                MemoryArena &arena,
                                                TransportMode mode,
                                                bool allowMultipleLobes) const {
    Spectrum r = _Kd->evaluate(*si).clamp();
    Float sig = _sigma ? clamp(_sigma->evaluate(*si), 0, 90) : 0;
    
    if (!r.IsBlack()) {
        int idx = getCurThreadIndex();
        auto bsdf = _bsdfs[idx];
        bsdf->updateGeometry(*si);
        bsdf->clearBxDFs();
        if (sig == 0) {
            auto lambert = (LambertianReflection *)bsdf->getBxDF(0);
            lambert->setReflection(r);
            bsdf->add(lambert);
        } else {
            auto lambert = (OrenNayar *)bsdf->getBxDF(1);
            lambert->setReflection(r);
            bsdf->add(lambert);
        }
        si->bsdf = bsdf.get();
    }
}



shared_ptr<const MatteMaterial> createLightMat() {
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
    
    nloJson _bumpMap = param.value("bumpMap", nloJson());
    auto bumpMap = shared_ptr<Texture<Float>>(createFloatTexture(_bumpMap));
    
    nloJson _normalMap = param.value("normalMap", nloJson());
    auto normalMap = shared_ptr<Texture<Spectrum>>(createSpectrumTexture(_normalMap));
    
    auto ret = new MatteMaterial(Kd, sigma, normalMap, bumpMap);
    return ret;
}

REGISTER("matte", createMatte)

PALADIN_END
