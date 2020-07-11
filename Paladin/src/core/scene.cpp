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

STAT_COUNTER("Intersections/rayIntersectTr ray intersection tests", nIntersectTrTests);
STAT_COUNTER("Scene/Finity lights", numFinityLights);
STAT_COUNTER("Scene/Sky lights", numSkyLights);

void Scene::initInfiniteLights() {
    for (auto i = 0; i < lights.size(); ++i) {
        auto &light = lights.at(i);
        light->preprocess(*this);
        if (light->flags & (int)LightFlags::Infinite) {
            infiniteLights.push_back(light);
            ++numFinityLights;
        }
        if (light->flags & (int)LightFlags::Env) {
            _envmap = static_cast<EnvironmentMap *>(light.get());
            ++numSkyLights;
            _envIndex = i;
        }
    }
}

bool Scene::rayIntersectTr(const Ray &r, Sampler &sampler, SurfaceInteraction *isect,
                        Spectrum *Tr) const {
    TRY_PROFILE(Prof::sceneRayIntersectTr)
    ++nIntersectTrTests;
    *Tr = Spectrum(1.f);
    Ray ray = r;
    while (true) {
        bool hitSurface = rayIntersect(ray, isect);
        if (ray.medium) {
            *Tr = ray.medium->Tr(ray, sampler);
        }
        
        if (!hitSurface) {
            return false;
        }
        
        if (isect->shape->getMaterial() != nullptr) {
            Float dist = (isect->pos - r.ori).length();
            Float t = dist / r.dir.length();
            r.tMax = t;
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
    return light->sample_Li(rcd, u, *this);
}



Spectrum Scene::sampleOneLight(ScatterSamplingRecord *scatterRcd,
                               MemoryArena &arena,
                               const Distribution1D *lightDistrib,
                               bool *foundIntersect, Spectrum *throughput,
                               bool handleMedia) const {
    TRY_PROFILE(Prof::DirectLighting)
    int nLights = int(lights.size());
    if (nLights == 0) {
        return Spectrum(0.0f);
    }
    
    int lightIndex;
    // 用于储存选中的光源的概率密度函数值
    Float lightPdf;
    Sampler &sampler = *scatterRcd->sampler;
    Point2f u = sampler.get2D();
    lightIndex = lightDistrib->sampleDiscrete(u[0], &lightPdf, &u[0]);
    if (lightPdf == 0) {
        return Spectrum(0.0f);
    }
    const Light * light = lights[lightIndex].get();
    DirectSamplingRecord rcd(scatterRcd->it);
    Spectrum dl = estimateDirectLighting(scatterRcd, arena, *light,&rcd, u,
                                         foundIntersect, throughput, handleMedia);
    return dl / lightPdf;
}

const Light * Scene::selectLight(Float *lightPmf,
                                 const Distribution1D *lightDistrib,
                                 Point2f &u) const {
    int nLights = int(lights.size());
    if (nLights == 0) {
        return nullptr;
    }
    
    int lightIndex;
    // 用于储存选中的光源的概率密度函数值
    lightIndex = lightDistrib->sampleDiscrete(u[0], lightPmf, &u[0]);
    if (*lightPmf == 0) {
        return nullptr;
    }
    const Light * light = lights[lightIndex].get();
    return light;
}

Spectrum Scene::nextEventEstimate(ScatterSamplingRecord *scatterRcd,
                                  MemoryArena &arena,
                                  const Distribution1D *lightDistrib,
                                  bool *foundIntersect,
                                  Spectrum *throughput,
                                  bool handleMedia) const {
    TRY_PROFILE(Prof::DirectLighting)
    Spectrum L(0.f);
    if (scatterRcd->it.isSurfaceInteraction()) {
        const SurfaceInteraction &isect = (const SurfaceInteraction &)scatterRcd->it;
        // sampling light surface
        const Light * light = nullptr;
        Float lightPmf = 0;
        BSDF * bsdf = isect.bsdf;
//        if (isect.bsdf->hasNonSpecular()) {
//            Point2f u = scatterRcd->sampler->get2D();
//            light = selectLight(&lightPmf, lightDistrib, u);
//            DirectSamplingRecord rcd(scatterRcd->it);
//
//            Spectrum Li = light->sample_Li(&rcd, u, *this);
//            Vector3f wi = rcd.dir();
//            Float lightPdf = rcd.pdfDir() * lightPmf;
//            BxDFType flags = BSDF_ALL;
//            Spectrum f = 0;
//            Float bsdfPdf = 0;
//            if (!Li.IsBlack() && lightPdf > 0) {
//                f = isect.bsdf->f(isect.wo, wi, flags);
//                f *= absDot(isect.shading.normal, wi);
//                bsdfPdf = isect.bsdf->pdfDir(isect.wo, wi);
//            }
//            if (!f.IsBlack()) {
//                Float weight = powerHeuristic(lightPdf, bsdfPdf);
//                L += Li * f * weight / lightPdf;
//            }
//        }
        DirectSamplingRecord rcd(isect);
        Sampler &sampler = *scatterRcd->sampler;
        if (bsdf->hasNonSpecular()) {
            Spectrum Ld = sampleLightDirect(&rcd, sampler.get2D(), lightDistrib, &lightPmf);
            light = static_cast<const Light *>(rcd.object);
            if (!Ld.IsBlack()) {
                Spectrum bsdfVal = bsdf->f(isect.wo, rcd.dir());
                bsdfVal *= absDot(isect.shading.normal, rcd.dir());
                if (!bsdfVal.IsBlack()) {
                    Float bsdfPdf = light->isDelta() ? 0 : bsdf->pdfDir(isect.wo, rcd.dir());
                    Float lightPdf = rcd.pdfDir() * lightPmf;
                    Float weight = bsdfPdf == 0 ? 1 : powerHeuristic(lightPdf, bsdfPdf);
                    L += *throughput * weight * bsdfVal * Ld / (lightPdf);
                }
            }
        }
        PROFILE(Prof::halfDL, __p)
        // sampling bsdf
        Vector3f wi;
        Float bsdfPdf;
        BxDFType sampledType;
        Spectrum f = isect.bsdf->sample_f(isect.wo, &wi,
                                          scatterRcd->sampler->get2D(),
                                          &bsdfPdf,
                                          BSDF_ALL,
                                          &sampledType);
        f *= absDot(isect.shading.normal, wi);
        scatterRcd->update(wi, f, bsdfPdf, sampledType, TransportMode::Radiance);
        
        if (bsdfPdf == 0 || f.IsBlack()) {
            return L;
        }
        *throughput *= f / bsdfPdf;
        Spectrum tr(1.0);
        SurfaceInteraction targetIsect;
        Ray ray = scatterRcd->it.spawnRay(wi);
        
//        DirectSamplingRecord rcd(scatterRcd->it);
        *foundIntersect = handleMedia ?
                rayIntersectTr(ray, *scatterRcd->sampler, &targetIsect, &tr):
                rayIntersect(ray, &targetIsect);
        scatterRcd->outRay = ray;
        
        if (*foundIntersect) {
            scatterRcd->nextIsect = targetIsect;
            rcd.updateTarget(targetIsect);
        } else {
            rcd.updateTarget(wi, 0);
        }
        
        if (light && !light->isDelta() && !f.IsBlack() && bsdfPdf > 0) {
            Spectrum Li(0.f);
            Float weight = 1;
            if (!scatterRcd->isSpecular()) {
                Float lightPdf = rcd.pdfDir() * lightPmf;
                weight = powerHeuristic(bsdfPdf, lightPdf);
            }
            if (*foundIntersect) {
                if (targetIsect.shape->getAreaLight() == light) {
                    Li = targetIsect.Le(-wi);
                }
            } else {
                Li = light->Le(ray);
            }
            if (!Li.IsBlack()) {
                auto tmp = Li * tr * f * weight / bsdfPdf;
                L += tmp;
            }
        }
    }
    return L;
}

Spectrum Scene::estimateDirectLighting(ScatterSamplingRecord *scatterRcd,
                                       MemoryArena &arena,
                                       const Light &light, DirectSamplingRecord *rcd,
                                       const Point2f &u,
                                       bool *foundIntersect, Spectrum *throughput,
                                       bool handleMedia) const {
    BxDFType bsdfFlags = BSDF_ALL;
    Spectrum Ld(0.0f);
    Float lightPdf = 0;
    Vector3f wi;
    Float scatteringPdf = 0;
    Spectrum scatterF;
    const Interaction &it = scatterRcd->it;
    Sampler &sampler = *scatterRcd->sampler;
    
    // 采样光源
    rcd->checkOccluded = false;
    Spectrum Li = light.sample_Li(rcd, u, *this);
    wi = rcd->dir();
    lightPdf = rcd->pdfDir();
    
    if (lightPdf > 0 && !Li.IsBlack()) {
        if (it.isSurfaceInteraction()) {
            const SurfaceInteraction &isect = (const SurfaceInteraction &)it;
            if (isect.bsdf->hasNonSpecular()) {
                scatterF = isect.bsdf->f(isect.wo, wi, bsdfFlags) *
                            absDot(wi, isect.shading.normal);
                scatteringPdf = isect.bsdf->pdfDir(isect.wo, wi);
            }
        } else {
            const MediumInteraction &mi = (const MediumInteraction &)it;
            Float scatteringPdf = mi.phase->p(mi.wo, wi);
            scatterF = Spectrum(scatteringPdf);
        }
        if (!scatterF.IsBlack()) {
            if (handleMedia) {
                Spectrum tr = rcd->Tr(*this, sampler);
                Li *= tr;
            } else if (!rcd->unoccluded(*this)){
                Li = Spectrum(0.f);
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
    bool sampledSpecular = false;
    BxDFType sampledType;
    
    if (it.isSurfaceInteraction()) {
        const SurfaceInteraction &isect = (const SurfaceInteraction &)it;
        scatterF = isect.bsdf->sample_f(isect.wo, &wi, sampler.get2D(),
                                        &scatteringPdf,
                                        bsdfFlags, &sampledType);
        scatterF *= absDot(wi, isect.shading.normal);
        sampledSpecular = (sampledType & BSDF_SPECULAR) != 0;
        
    } else {
        const MediumInteraction &mi = (const MediumInteraction &)it;
        scatteringPdf = mi.phase->sample_p(mi.wo, &wi, sampler.get2D());
        scatterF = Spectrum(scatteringPdf);
    }
    
    scatterRcd->update(wi, scatterF, scatteringPdf, sampledType, Radiance);
    if (scatteringPdf == 0 || scatterF.IsBlack()) {
        return Ld;
    }
    *throughput *= scatterF / scatteringPdf;
    
    
    Spectrum tr(1.0);
    SurfaceInteraction targetIsect;
    Ray ray = it.spawnRay(wi);
    
    *foundIntersect = handleMedia ?
                    rayIntersectTr(ray, sampler, &targetIsect, &tr):
                    rayIntersect(ray, &targetIsect);
    
    scatterRcd->outRay = ray;
    
    if (*foundIntersect) {
        scatterRcd->nextIsect = targetIsect;
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
    ray.tMax = rh.ray.tfar;
    
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
