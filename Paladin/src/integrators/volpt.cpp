//
//  volpath.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/21.
//

#include "volpt.hpp"
#include "core/camera.hpp"
#include "core/medium.hpp"
#include "materials/bxdfs/bsdf.hpp"
#include "core/shape.hpp"

PALADIN_BEGIN

STAT_INT_DISTRIBUTION("Integrator/Path length", pathLength);

VolumePathTracer::VolumePathTracer(int maxDepth, std::shared_ptr<const Camera> camera,
                       std::shared_ptr<Sampler> sampler,
                       const AABB2i &pixelBounds, Float rrThreshold /* = 1*/,
                       const std::string &lightSampleStrategy /*= "power"*/)
: MonteCarloIntegrator(camera, sampler, pixelBounds),
_maxDepth(maxDepth),
_rrThreshold(rrThreshold),
_lightSampleStrategy(lightSampleStrategy) {
    
}

void VolumePathTracer::preprocess(const Scene &scene, Sampler &sampler) {
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

Spectrum VolumePathTracer::_Li(const RayDifferential &r, const Scene &scene,
                              Sampler &sampler, MemoryArena &arena, int depth) const {
    Spectrum L(0.f);
    Spectrum throughput(1.f);
    RayDifferential ray(r);
    bool specularBounce = false;
    int bounces;

    Float etaScale = 1.f;
    SurfaceInteraction isect;
    bool foundIntersection;
    foundIntersection = scene.rayIntersect(ray, &isect);
    
    for (bounces = 0; ; ++bounces) {
        MediumInteraction mi;
        if (ray.medium) {
            // 如果ray有参与介质，则对参与介质进行采样，计算散射
            throughput *= ray.medium->sample(ray, sampler, arena, &mi);
        }
        if (throughput.IsBlack()) {
            break;
        }
        
        if (mi.isValid()) {
            // 如果采样点落在参与介质中
            // 其实跟采样表面一样的处理方式，
            // 1.估计直接光照
            // 2.随机选方向继续追踪
            if (bounces >= _maxDepth) {
                break;
            }
            
            // 采样光源
            Vector3f wo = -ray.dir;
            DirectSamplingRecord rcd(mi);
            const Distribution1D * distrib = _lightDistribution->lookup(mi.pos);
            const Light * light = nullptr;
            Float pmf = 0;
            Spectrum Li = scene.sampleLightDirect(&rcd, sampler.get2D(), distrib, &pmf);
            light = static_cast<const Light *>(rcd.object);
            if (!Li.IsBlack()) {
                Spectrum tr = rcd.Tr(scene, sampler);
                // todo 这里可以优化
                Li *= tr;
                Float scatterPdf = mi.phase->p(wo, rcd.dir());
                Spectrum scatterF(scatterPdf);
                Float lightPdf = rcd.pdfDir();
                if (light->isDelta()) {
                    L += scatterF * Li / lightPdf;
                } else {
                    Float weight = powerHeuristic(lightPdf, scatterPdf);
                    L += Li * scatterF * weight / lightPdf;
                }
            }
            
            // 采样phase函数
            Vector3f wi;
            SurfaceInteraction targetIsect;
            Float scatterPdf = mi.phase->sample_p(mi.wo, &wi, sampler.get2D());
            Spectrum scatterF(scatterPdf), tr(1.f);
            ray = mi.spawnRay(wi);
            
            foundIntersection = scene.rayIntersectTr(ray, sampler, &targetIsect, &tr);
            if (foundIntersection) {
                if (light && light == targetIsect.shape->getAreaLight()) {
                    rcd.updateTarget(targetIsect);
                    Spectrum Le = targetIsect.Le(-wi);
                    Float lightPdf = rcd.pdfDir();
                    Float weight = powerHeuristic(scatterPdf, lightPdf);
                    L += Le * tr * scatterF * weight / scatterPdf;
                }
            }
            specularBounce = false;
            
        } else {
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

            isect.computeScatteringFunctions(ray, arena, true);
            if (!isect.bsdf) {
                ray = isect.spawnRay(ray.dir, true);
                foundIntersection = scene.rayIntersect(ray, &isect);
                --bounces;
                continue;
            }
            
            BSDF * bsdf = isect.bsdf;
            
            // 采样光源
            const Distribution1D * distrib = _lightDistribution->lookup(isect.pos);
            DirectSamplingRecord rcd(isect);
            const Light * light = nullptr;
            Float pmf = 0;
            
            if (bsdf->hasNonSpecular()) {
                Spectrum Li = scene.sampleLightDirect(&rcd, sampler.get2D(), distrib, &pmf);
                light = static_cast<const Light *>(rcd.object);
                if (!Li.IsBlack()) {
                    Spectrum tr = rcd.Tr(scene, sampler);
                    Li *= tr;
                    Spectrum bsdfVal = bsdf->f(isect.wo, rcd.dir());
                    bsdfVal *= absDot(isect.shading.normal, rcd.dir());
                    if (!bsdfVal.IsBlack()) {
                        Float bsdfPdf = light->isDelta() ? 0 : bsdf->pdfDir(isect.wo, rcd.dir());
                        Float lightPdf = rcd.pdfDir() * pmf;
                        Float weight = bsdfPdf == 0 ? 1 : powerHeuristic(lightPdf, bsdfPdf);
                        L += throughput * weight * bsdfVal * Li / (lightPdf);
                    }
                }
            }

            // 采样bsdf
            Vector3f wo = -ray.dir;
            Vector3f wi;
            Float bsdfPdf;
            BxDFType flags;
            Spectrum f = bsdf->sample_f(wo, &wi, sampler.get2D(),
                                        &bsdfPdf, BSDF_ALL, &flags);
            
            if (bsdfPdf == 0 || f.IsBlack()) {
                break;
            }
            f *= absDot(wi, isect.shading.normal);

            specularBounce = (flags & BSDF_SPECULAR) != 0;
            
            if ((flags & BSDF_TRANSMISSION)) {
                Float eta = bsdf->eta;
                // 详见bxdf.hpp文件中SpecularTransmission的注释
                etaScale *= (dot(wo, isect.normal) > 0) ? (eta * eta) : 1 / (eta * eta);
            }
            ray = isect.spawnRay(wi);
            Spectrum tr(1.f);
            SurfaceInteraction targetIsect;
            foundIntersection = scene.rayIntersectTr(ray, sampler, &targetIsect, &tr);
            Spectrum Li(0.f);
            Float lightPdf = 0;
            Float weight = 1;
            
            if (foundIntersection) {
                if (light && !light->isDelta()) {
                    rcd.updateTarget(isect);
                    const Light * target = isect.shape->getAreaLight();
                    if (target == light) {
                        Li = isect.Le(-wi);
                        lightPdf = rcd.pdfDir() * pmf;
                        if (!specularBounce) {
                            weight = powerHeuristic(bsdfPdf, lightPdf);
                        }
                        if (bsdf->hasNonSpecular()) {
                            L += Li.IsBlack() ? 0 : f * throughput * Li * tr * weight / bsdfPdf;
                        }
                    }
                }
            } else {
                Li = scene.evalEnvironment(ray, &lightPdf, distrib);
                if (!specularBounce) {
                    weight = powerHeuristic(bsdfPdf, lightPdf);
                }
                if (bsdf->hasNonSpecular()) {
                    L += Li.IsBlack() ? 0 : f * throughput * Li * tr* weight / bsdfPdf;
                }
            }
        }
        // 为何不直接使用throughput，包含的是radiance，radiance是经过折射缩放的
        // 但rrThroughput没有经过折射缩放，包含的是power，我们需要根据能量去筛选路径
        Spectrum rrThroughput = throughput * etaScale;
        Float mp = rrThroughput.MaxComponentValue();
        if (mp < _rrThreshold && bounces > 3) {
            Float q = std::min((Float)0.95, mp);
            if (sampler.get1D() >= q) {
                break;
            }
            throughput /= q;
            DCHECK(!std::isinf(throughput.y()));
        }
    }

    
    ReportValue(pathLength, bounces);
    return L;
}


/**
 * 体渲染路径追踪跟普通路径追踪的区别主要在于
 * 如果场景中有介质，则对介质进行采样
 *      如果采样到物体表面，则按照传统pt算法继续执行
 *      如果采样到介质，则生成MediumInteraction对象，再随机采样相函数生成新的ray对象
 * 
 */
Spectrum VolumePathTracer::Li(const RayDifferential &r, const Scene &scene,
						Sampler &sampler, MemoryArena &arena, int depth) const {
    
    return _Li(r, scene, sampler, arena, depth);
    
    Spectrum L(0.f);
    Spectrum throughput(1.f);
    RayDifferential ray(r);
    bool specularBounce = false;
    int bounce;

    Float etaScale = 1.f;
    SurfaceInteraction isect;
    bool foundIntersection;
    foundIntersection = scene.rayIntersect(ray, &isect);
    for (bounce = 0; ; ++bounce) {
        
        MediumInteraction mi;
        if (ray.medium) {
            // 如果ray有参与介质，则对参与介质进行采样，计算散射
            throughput *= ray.medium->sample(ray, sampler, arena, &mi);
        }
        if (throughput.IsBlack()) {
            break;
        }
        
        if (mi.isValid()) {
            // 如果采样点落在参与介质中
            // 其实跟采样表面一样的处理方式，
            // 1.估计直接光照
            // 2.随机选方向继续追踪
            if (bounce >= _maxDepth) {
                break;
            }
            
            Vector3f wo = -ray.dir;
            Spectrum tmpThroughput = throughput;
            ScatterSamplingRecord scatterRcd(mi, &sampler);
            
            const Distribution1D *lightDistrib = _lightDistribution->lookup(mi.pos);
            L += throughput * scene.sampleOneLight(&scatterRcd, arena, lightDistrib,
                                                   &foundIntersection, &tmpThroughput, true);
            
            
            ray = scatterRcd.outRay;
            specularBounce = false;
            if (foundIntersection) {
                isect = scatterRcd.nextIsect;
            }
            
        } else {
            // 如果采样到的顶点没有落在物体表面
            // 如果当前ray是直接从相机发射，
            // 判断光线是否与场景几何图元相交
            if (specularBounce || bounce == 0) {
                // 因为在这种情况下，ray的方向是确定的，因此直接取isect的Le
                if (foundIntersection) {
                    L += throughput * isect.Le(-ray.dir);
                } else {
                    for (const auto &light : scene.infiniteLights) {
                        L += throughput * light->Le(ray);
                    }
                }
            }

            if (!foundIntersection || bounce > _maxDepth) {
                break;
            }
            // 计算bsdf
            isect.computeScatteringFunctions(ray, arena, true);
            // 如果没有bsdf，则不计算反射次数
            // 有些几何图元是仅仅是为了限定参与介质的范围
            // 所以没有bsdf
            if (!isect.bsdf) {
                ray = isect.spawnRay(ray.dir);
                --bounce;
                foundIntersection = scene.rayIntersect(ray, &isect);
                continue;
            }
            const Distribution1D * distrib = _lightDistribution->lookup(isect.pos);

            Vector3f wo = -ray.dir;
            Float pdf;
            BxDFType flags;
            Spectrum f;
            Spectrum tmpThroughput = throughput;
            ScatterSamplingRecord scatterRcd(isect, &sampler);
            
            Spectrum Ld = tmpThroughput * scene.sampleOneLight(&scatterRcd, arena, distrib,
                                                               &foundIntersection, &throughput, true);
            
            if (isect.bsdf->hasNonSpecular()) {
                L += Ld;
            }
            
        
            pdf = scatterRcd.pdf;
            f = scatterRcd.scatterF;
            specularBounce = scatterRcd.isSpecular();
            flags = scatterRcd.sampleType;
            
            ray = scatterRcd.outRay;
            if (pdf == 0 || f.IsBlack()) {
                break;
            }

            CHECK_GE(throughput.y(), 0.0f);
            DCHECK(!std::isinf(throughput.y()));
            
            if (flags & BSDF_TRANSMISSION) {
                // 如果采样到投射comp
                Float eta = isect.bsdf->eta;
                // 详见bxdf.hpp文件中SpecularTransmission的注释
                etaScale *= (dot(wo, isect.normal) > 0) ? (eta * eta) : 1 / (eta * eta);
            }
            
            if (isect.bssrdf && (flags & BSDF_TRANSMISSION)) {
                // todo 处理bssrdf
            }
            if (foundIntersection) {
                isect = scatterRcd.nextIsect;
            }
        }
        
        // 为何不直接使用throughput，包含的是radiance，radiance是经过折射缩放的
        // 但rrThroughput没有经过折射缩放，包含的是power，我们需要根据能量去筛选路径
        Spectrum rrThroughput = throughput * etaScale;
        Float mp = rrThroughput.MaxComponentValue();
        if (mp < _rrThreshold && bounce > 3) {
            Float q = std::min((Float)0.05, mp);
            if (sampler.get1D() >= q) {
                break;
            }
            throughput /= q;
            DCHECK(!std::isinf(throughput.y()));
        }
    }
    ReportValue(pathLength, bounce);
    return L;
}

//"param" : {
//    "maxBounce" : 5,
//    "rrThreshold" : 1,
//    "lightSampleStrategy" : "power"
//}
// lst = {sampler, camera}
CObject_ptr createVolumePathTracer(const nloJson &param, const Arguments &lst) {
    int maxBounce = param.value("maxBounce", 5);
    Float rrThreshold = param.value("rrThreshold", 1.f);
    string lightSampleStrategy = param.value("lightSampleStrategy", "power");
    auto iter = lst.begin();
    Sampler * sampler = dynamic_cast<Sampler *>(*iter);
    ++iter;
    Camera * camera = dynamic_cast<Camera *>(*iter);
    AABB2i pixelBounds = camera->film->getSampleBounds();
    VolumePathTracer * ret = new VolumePathTracer(maxBounce,
                                      shared_ptr<const Camera>(camera),
                                      shared_ptr<Sampler>(sampler),
                                      pixelBounds,
                                      rrThreshold,
                                      lightSampleStrategy);
    
    return ret;
}

REGISTER("volpt", createVolumePathTracer);


PALADIN_END
