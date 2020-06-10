//
//  scene.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "scene.hpp"
#include "tools/embree_util.hpp"
#include "shapes/mesh.hpp"
#include "core/sampler.hpp"
#include "materials/bxdfs/bsdf.hpp"

PALADIN_BEGIN

void Scene::initInfiniteLights() {
    for (auto i = 0; i < lights.size(); ++i) {
        auto &light = lights.at(i);
        light->preprocess(*this);
        if (light->flags & (int)LightFlags::Infinite) {
            infiniteLights.push_back(light);
        }
        if (light->flags & (int)LightFlags::Env) {
            _envmap = static_cast<EnvironmentMap *>(light.get());
            _envIndex = i;
        }
    }
}

bool Scene::rayIntersectTr(Ray ray, Sampler &sampler, SurfaceInteraction *isect,
                        Spectrum *Tr) const {
    *Tr = Spectrum(1.f);
    while (true) {
        bool hitSurface = rayIntersect(ray, isect);
        if (ray.medium) {
            *Tr = ray.medium->Tr(ray, sampler);
        }
        
        if (!hitSurface) {
            return false;
        }
        
        if (isect->shape->getMaterial() != nullptr) {
            return true;
        }
        ray = isect->spawnRay(ray.dir);
    }
}

void Scene::initAccel(const nloJson &data, const vector<shared_ptr<const Shape> > &shapes) {
    string type = data.value("type", "embree");
    _shapes = shapes;
    if (type == "embree") {
        InitAccelEmbree(shapes);
    } else {
        InitAccelNative(data, shapes);
    }
}

Spectrum Scene::sampleLightDirect(DirectSamplingRecord *rcd, const Point2f _u,
                                  const Distribution1D *lightDistrib,
                                  Float *pmf) const {
    Point2f u(_u);
    Float index = lightDistrib->sampleDiscrete(u.x, pmf, &u.x);
    const Light * light = lights.at(index).get();
    rcd->object = light;
//    Vector3f wi;
//    Float pdf;
//    VisibilityTester vis;
//    auto r = light->sample_Li(Interaction(rcd->ref()), u, &wi, &pdf, &vis);
    auto r2 = light->sample_Li(rcd, u, *this);
    return r2;
}

Spectrum Scene::sampleOneLight(const Interaction &it, MemoryArena &arena,
                               Sampler &sampler,
                               const Distribution1D *lightDistrib,
                               bool *foundIntersect, Float *pdf,
                               bool *specular, Spectrum *throughput,
                               Spectrum *scatterF,
                               Vector3f *wi, bool handleMedia) const {
    int nLights = int(lights.size());
    if (nLights == 0) {
        return Spectrum(0.0f);
    }
    
    int lightIndex;
    // 用于储存选中的光源的概率密度函数值
    Float lightPdf;
    lightIndex = lightDistrib->sampleDiscrete(sampler.get1D(), &lightPdf);
    if (lightPdf == 0) {
        return Spectrum(0.0f);
    }
    const Light * light = lights[lightIndex].get();
    DirectSamplingRecord rcd(it);
    Spectrum dl = estimateDirectLighting(it, arena, sampler, *light,
                                         throughput, foundIntersect,
                                         &rcd, scatterF,
                                         specular, lightDistrib,
                                         handleMedia);
    *wi = rcd.dir();
    *pdf = rcd.pdfDir();
    return dl / lightPdf;
}

Spectrum Scene::estimateDirectLighting(const Interaction &it,
                                       MemoryArena &arena,
                                       Sampler &sampler,
                                       const Light &light,
                                       Spectrum *throughput,
                                       bool *foundIntersect,
                                       DirectSamplingRecord *rcd,
                                       Spectrum *f,
                                       bool *specular,
                                       const Distribution1D *lightDistrib,
                                       bool handleMedia) const {
    BxDFType bsdfFlags = BSDF_ALL;
    Spectrum Ld(0.0f);
    Float lightPdf = 0;
    Vector3f wi;
    Float scatteringPdf = 0;
    Spectrum scatterF;
    
    // 采样光源
    Spectrum Li = light.sample_Li(rcd, sampler.get2D(), *this);
    wi = rcd->dir();
    lightPdf = rcd->pdfDir();
    
    if (rcd->pdfDir() > 0 && ! Li.IsBlack()) {
        if (it.isSurfaceInteraction()) {
            const SurfaceInteraction &isect = (const SurfaceInteraction &)it;
            scatterF = isect.bsdf->f(isect.wo, wi, bsdfFlags) * absDot(wi, isect.shading.normal);
            scatteringPdf = isect.bsdf->pdfDir(isect.wo, wi);
        } else {
            const MediumInteraction &mi = (const MediumInteraction &)it;
            Float scatteringPdf = mi.phase->p(mi.wo, wi);
            scatterF = Spectrum(scatteringPdf);
        }
        if (!scatterF.IsBlack()) {
            if (handleMedia) {
                Spectrum tr = rcd->Tr(*this, sampler);
                Li *= tr;
            } else {
                if (!rcd->unoccluded(*this)) {
                    Li = Spectrum(0.f);
                }
            }
            if (!Li.IsBlack()) {
                // 如果是delta分布，直接计算辐射度
                if (light.isDelta()) {
                    Ld += scatterF * Li / lightPdf;
                } else {
                    Float weight = powerHeuristic(lightPdf, scatteringPdf);
                    Ld += scatterF * Li * weight / lightPdf;
                }
            }
        }
    }

    Li = Spectrum(0.f);
    // 对bsdf进行随机采样，向外返回wi,pdf，下次循环复用
    bool sampledSpecular = false;
    if (it.isSurfaceInteraction()) {
        BxDFType sampledType;
        const SurfaceInteraction &isect = (const SurfaceInteraction &)it;
        scatterF = isect.bsdf->sample_f(isect.wo, &wi, sampler.get2D(),
                                        &scatteringPdf,
                                        bsdfFlags, &sampledType);
        scatterF *= absDot(wi, isect.shading.normal);
        sampledSpecular = (sampledType & BSDF_SPECULAR) != 0;
        *throughput *= scatterF / scatteringPdf;
    } else {
        const MediumInteraction &mi = (const MediumInteraction &)it;
        scatteringPdf = mi.phase->sample_p(mi.wo, &wi, sampler.get2D());
        scatterF = Spectrum(scatteringPdf);
    }
    *specular = sampledSpecular;
    Ray ray = it.spawnRay(wi);
    Spectrum tr(1.0);
    SurfaceInteraction targetIsect;
    *f = scatterF;
    *foundIntersect = handleMedia ?
                    rayIntersectTr(ray, sampler, &targetIsect, &tr):
                    rayIntersect(ray, &targetIsect);

    if (*foundIntersect) {
        rcd->updateTarget(targetIsect);
    } else {
        rcd->updateTarget(wi, 0);
    }
    lightPdf = rcd->pdfDir();

    if (!light.isDelta() && !scatterF.IsBlack() && scatteringPdf > 0) {
        Float weight = 1;
        if (!sampledSpecular) {
            weight = powerHeuristic(scatteringPdf, lightPdf);
        }
        if (*foundIntersect) {
            if (targetIsect.shape->getAreaLight() == &light) {
                Li = targetIsect.Le(-wi);
            }
        } else {
            Li = light.Le(ray);
        }
        if (!Li.IsBlack()) {
            auto tmp = Li * tr * scatterF * weight / scatteringPdf;
            Ld += tmp;
        }
    }
    
    return Ld;
}

bool Scene::rayIntersectEmbree(const Ray &ray, SurfaceInteraction *isect) const {
    RTCIntersectContext context;
    rtcInitIntersectContext(&context);
    RTCRayHit rh = EmbreeUtil::toRTCRayHit(ray);
    rtcIntersect1(_rtcScene, &context, &rh);
    if (rh.hit.geomID == RTC_INVALID_GEOMETRY_ID) {
        return false;
    }
    uint32_t gid = rh.hit.geomID;
    uint32_t pid = rh.hit.primID;
    Vector2f uv(rh.hit.u, rh.hit.v);
    
    const Shape * shape = _shapes.at(gid).get();
    switch (shape->getType()) {
        case EMesh: {
            auto mesh = static_cast<const Mesh *>(shape);
            auto tri = mesh->getTriangle(pid);
            tri->fillSurfaceInteraction(ray, uv, isect);
            break;
        }
        default:
            break;
    }
    return true;
}

void Scene::InitAccelNative(const nloJson &data, const vector<shared_ptr<const Shape>> &shapes) {
    
    _aggregate = createAccelerator(data, shapes);
    _worldBound = _aggregate->worldBound();
    initInfiniteLights();
}

void Scene::InitAccelEmbree(const vector<shared_ptr<const Shape>>&shapes) {
    EmbreeUtil::initDevice();
    _rtcScene = rtcNewScene(EmbreeUtil::getDevice());
    int idx = 0;
    for (size_t i = 0; i < shapes.size(); ++i) {
        auto prim = shapes[i];
        RTCGeometry gid = prim->rtcGeometry(this);
        _worldBound = unionSet(_worldBound, prim->worldBound());
        if (gid != nullptr) {
            rtcAttachGeometry(_rtcScene, gid);
        }
    }
    initInfiniteLights();
    rtcCommitScene(_rtcScene);
}

PALADIN_END
