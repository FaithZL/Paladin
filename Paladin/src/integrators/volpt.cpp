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

PALADIN_BEGIN

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
    for (const auto &light : scene.lights) {
        _nLightSamples.push_back(sampler.roundCount(light->nSamples));
    }
    for (int i = 0; i < _maxDepth; ++i) {
        for (size_t j = 0; j < scene.lights.size(); ++j) {
            sampler.request2DArray(_nLightSamples[j]);
            sampler.request2DArray(_nLightSamples[j]);
        }
    }
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
    Spectrum L(0.f);
    Spectrum throughput(1.f);
    RayDifferential ray(r);
    bool specularBounce = false;
    int bounce;

    Float etaScale = 1.f;

    for (bounce = 0; ; ++bounce) {
        SurfaceInteraction isect;
        bool foundIntersection = scene.intersect(ray, &isect);
        
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
            const Distribution1D *lightDistrib = _lightDistribution->lookup(mi.pos);
            L += throughput * sampleOneLight(mi, scene, arena, sampler, _nLightSamples, true,
                                              lightDistrib);
            
            Vector3f wo = -ray.dir;
            Vector3f wi;
            mi.phase->sample_p(wo, &wi, sampler.get2D());
            ray = mi.spawnRay(wi);
            specularBounce = false;
            
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
            isect.computeScatteringFunctions(ray, arena);
            // 如果没有bsdf，则不计算反射次数
            // 有些几何图元是仅仅是为了限定参与介质的范围
            // 所以没有bsdf
            if (!isect.bsdf) {
                ray = isect.spawnRay(ray.dir);
                --bounce;
                continue;
            }
            const Distribution1D * distrib = _lightDistribution->lookup(isect.pos);
            // 找到非高光反射comp，如果有，则估计直接光照贡献
            if (isect.bsdf->numComponents(BxDFType(BSDF_ALL & ~BSDF_SPECULAR))) {
                Spectrum Ld = throughput * sampleOneLight(isect, scene, arena,
                                                 sampler, _nLightSamples, true, distrib);
                L += Ld;
            }
            Vector3f wo = -ray.dir;
            Vector3f wi;
            Float pdf;
            BxDFType flags;
            Spectrum f = isect.bsdf->sample_f(wo, &wi, sampler.get2D(), &pdf, BSDF_ALL, &flags);
            ray = isect.spawnRay(wi);
            if (f.IsBlack() || pdf == 0) {
                break;
            }
            /**
             * 复用之前的路径对吞吐量进行累积
             *      f(pj+1 → pj → pj-1) |cosθj|
             *    --------------------------------
             *             pω(pj+1 - pj)
             */
            throughput *= f * absDot(wi, isect.shading.normal) / pdf;
            CHECK_GE(throughput.y(), 0.0f);
            DCHECK(!std::isinf(throughput.y()));
            specularBounce = (flags & BSDF_SPECULAR) != 0;
            
            if (flags & BSDF_TRANSMISSION) {
                // 如果采样到投射comp
                Float eta = isect.bsdf->eta;
                // 详见bxdf.hpp文件中SpecularTransmission的注释
                etaScale *= (dot(wo, isect.normal) > 0) ? (eta * eta) : 1 / (eta * eta);
            }
            
            ray = isect.spawnRay(wi);
            if (isect.bssrdf && (flags & BSDF_TRANSMISSION)) {
                // todo 处理bssrdf
            }
        }
        // 为何不直接使用throughput，包含的是radiance，radiance是经过折射缩放的
        // 但rrThroughput没有经过折射缩放，包含的是power，我们需要根据能量去筛选路径
        Spectrum rrThroughput = throughput * etaScale;
        if (rrThroughput.MaxComponentValue() < _rrThreshold && bounce > 3) {
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
