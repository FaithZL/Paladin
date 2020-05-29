//
//  pathtracer.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "pathtracer.hpp"
#include "core/camera.hpp"
#include "materials/bxdfs/bsdf.hpp"
#include "core/shape.hpp"

PALADIN_BEGIN

PathTracer::PathTracer(int maxDepth, std::shared_ptr<const Camera> camera,
                       std::shared_ptr<Sampler> sampler,
                       const AABB2i &pixelBounds, Float rrThreshold /* = 1*/,
                       const std::string &lightSampleStrategy /*= "power"*/)
: MonteCarloIntegrator(camera, sampler, pixelBounds),
_maxDepth(maxDepth),
_rrThreshold(rrThreshold),
_lightSampleStrategy(lightSampleStrategy) {
    
}

void PathTracer::preprocess(const Scene &scene, Sampler &sampler) {
    _lightDistribution = createLightSampleDistribution(_lightSampleStrategy, scene);
//    for (const auto &light : scene.lights) {
//        _nLightSamples.push_back(sampler.roundCount(light->nSamples));
//    }
//    for (int i = 0; i < _maxDepth; ++i) {
//        for (size_t j = 0; j < scene.lights.size(); ++j) {
//            sampler.request2DArray(_nLightSamples[j]);
//            sampler.request2DArray(_nLightSamples[j]);
//        }
//    }
}

Spectrum PathTracer::Li(const RayDifferential &r, const Scene &scene,
                         Sampler &sampler, MemoryArena &arena, int depth) const {
    Spectrum L(0.0f);
    Spectrum throughput(1.0f);
    RayDifferential ray(r);
    bool specularBounce = false;
    int bounces;
    
    Float etaScale = 1;
    
    SurfaceInteraction isect;
    bool foundIntersection = scene.rayIntersect(ray, &isect);

    for (bounces = 0; ; ++bounces) {
        
        if (bounces == 0 || specularBounce) {
            if (foundIntersection) {
                L += throughput * isect.Le(-ray.dir);
            } else {
                for (const auto &light : scene.infiniteLights) {
                    L += throughput * light->Le(ray);
                }
            }
        }
        
        if (!foundIntersection || bounces >= _maxDepth) {
            break;
        }
        
        isect.computeScatteringFunctions(ray, arena);
        
        if (!isect.bsdf) {
            ray = isect.spawnRay(ray.dir);
            --bounces;
            continue;
        }
        
        // 采样光源
        const Distribution1D * distrib = _lightDistribution->lookup(isect.pos);
        DirectSamplingRecord rcd(isect);
        const Light * light = nullptr;
        if (isect.bsdf->numComponents(BxDFType(BSDF_ALL & ~BSDF_SPECULAR)) > 0) {
            Float pmf = 0;
            Spectrum Ld = scene.sampleLightDirect(&rcd, sampler.get2D(), distrib, &pmf);
            if (!Ld.IsBlack()) {
                Spectrum bsdfVal = isect.bsdf->f(isect.wo, rcd.dir());
                bsdfVal *= absDot(isect.shading.normal, rcd.dir());
                if (!bsdfVal.IsBlack()) {
                    light = static_cast<const Light *>(rcd.object);
                    Float bsdfPdf = light->isDelta() ? 0 : isect.bsdf->pdfDir(isect.wo, rcd.dir());
                    Float weight = bsdfPdf == 0 ? 1 : powerHeuristic(rcd.pdfDir(), bsdfPdf);
                    L += throughput * weight * bsdfVal * Ld / (rcd.pdfDir() * pmf);
                }
            }
        }
        
        // 采样bsdf
        Vector3f wo = -ray.dir;
        Vector3f wi;
        Float bsdfPdf;
        BxDFType flags;
        Spectrum f = isect.bsdf->sample_f(wo, &wi, sampler.get2D(), &bsdfPdf, BSDF_ALL, &flags);
        if (bsdfPdf == 0 || f.IsBlack()) {
            break;
        }
        f *= absDot(wi, isect.shading.normal);
        
        specularBounce = (flags & BSDF_SPECULAR) != 0;
        if ((flags & BSDF_TRANSMISSION)) {
            Float eta = isect.bsdf->eta;
            // 详见bxdf.hpp文件中SpecularTransmission的注释
            etaScale *= (dot(wo, isect.normal) > 0) ? (eta * eta) : 1 / (eta * eta);
        }
        ray = isect.spawnRay(wi);
        foundIntersection = scene.rayIntersect(ray, &isect);
        if (foundIntersection) {
            rcd.updateTarget(isect);
            Spectrum Li(0.f);
            const Light * target = isect.shape->getAreaLight();
            if (target && target == light) {
                Li = isect.Le(-wi);
                Float lightPdf = rcd.pdfDir();
                Float weight = powerHeuristic(bsdfPdf, lightPdf);
                L += throughput * Li * weight / bsdfPdf;
            }
        }
        throughput *= f / bsdfPdf;
        
        Spectrum rrThroughput = throughput * etaScale;
        if (rrThroughput.MaxComponentValue() < _rrThreshold && bounces > 3) {
            Float q = std::max((Float)0.05, 1 - rrThroughput.MaxComponentValue());
            if (sampler.get1D() < q) {
                break;
            }
            throughput /= 1 - q;
            DCHECK(!std::isinf(throughput.y()));
        }
    }
    return L;
}


USING_STD;

//"param" : {
//    "maxBounce" : 5,
//    "rrThreshold" : 1,
//    "lightSampleStrategy" : "power"
//}
// lst = {sampler, camera}
CObject_ptr createPathTracer(const nloJson &param, const Arguments &lst) {
    int maxBounce = param.value("maxBounce", 5);
    Float rrThreshold = param.value("rrThreshold", 1.f);
    string lightSampleStrategy = param.value("lightSampleStrategy", "power");
    auto iter = lst.begin();
    Sampler * sampler = dynamic_cast<Sampler *>(*iter);
    ++iter;
    Camera * camera = dynamic_cast<Camera *>(*iter);
    AABB2i pixelBounds = camera->film->getSampleBounds();
    PathTracer * ret = new PathTracer(maxBounce,
                                      shared_ptr<const Camera>(camera),
                                      shared_ptr<Sampler>(sampler),
                                      pixelBounds,
                                      rrThreshold,
                                      lightSampleStrategy);
    
    return ret;
}

REGISTER("pt", createPathTracer);



PALADIN_END
