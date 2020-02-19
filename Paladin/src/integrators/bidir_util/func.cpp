//
//  func.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/2/7.
//

#include "func.hpp"
#include "core/scene.hpp"
#include "math/sampling.hpp"
#include "core/sampler.hpp"
#include "core/bxdf.hpp"
#include "materials/bxdfs/bsdf.hpp"

PALADIN_BEGIN

Float infiniteLightPdf(
    const Scene &scene, const Distribution1D &lightDistr,
    const std::unordered_map<const Light *, size_t> &lightToDistrIndex,
    const Vector3f &w) {
    Float pdf = 0;
    for (const auto &light : scene.infiniteLights) {
        CHECK(lightToDistrIndex.find(light.get()) != lightToDistrIndex.end());
        size_t index = lightToDistrIndex.find(light.get())->second;
        pdf += light->pdf_Li(Interaction(), -w) * lightDistr.funcAt(index);
    }
    // pdf需要归一化
    return pdf / (lightDistr.getFuncInt() * lightDistr.count());
}

Float correctShadingNormal(const SurfaceInteraction &isect, const Vector3f &wo,
                           const Vector3f &wi, TransportMode mode) {
    if (mode == TransportMode::Importance) {
        Float num = absDot(wo, isect.shading.normal) * absDot(wi, isect.normal);
        Float denom = absDot(wo, isect.normal) * absDot(wi, isect.shading.normal);
        if (denom == 0) {
            return 0;
        }
        return num / denom;
    }
    return 1;
}

/**
 * 原函数为
 *                    |cosθ| |cosθ'|
 *      G(p'↔p) =  -------------------- V(p'↔p)
 *                      |p - p'|^2
 *
 * 如果顶点在medium中，则不需要乘以cos
 * 
 * @param  scene   [description]
 * @param  sampler [description]
 * @param  v0      [description]
 * @param  v1      [description]
 * @return         [description]
 */
Spectrum G(const Scene &scene, Sampler &sampler, const Vertex &v0,
           const Vertex &v1) {
    Vector3f d = v0.pos() - v1.pos();
    Float g = 1 / d.lengthSquared();
    d *= std::sqrt(g);
    if (v0.isOnSurface()) {
        g *= absDot(v0.ns(), d);
    }
    if (v1.isOnSurface()) {
        g *= absDot(v1.ns(), d);
    }
    VisibilityTester vis(v0.getInteraction(), v1.getInteraction());
    return g * vis.Tr(scene, sampler);
}

int generateCameraSubpath(const Scene &scene, Sampler &sampler,
                            MemoryArena &arena, int maxDepth,
                            const Camera &camera, const Point2f &pFilm,
                          Vertex *path) {
    if (maxDepth == 0)
        return 0;
    CameraSample cameraSample;
    cameraSample.pFilm = pFilm;
    cameraSample.time = sampler.get1D();
    cameraSample.pLens = sampler.get2D();
    
    RayDifferential ray;
    Spectrum throughput = camera.generateRayDifferential(cameraSample, &ray);
    ray.scaleDifferentials(1 / std::sqrt(sampler.samplesPerPixel));
    
    Float pdfPos, pdfDir;
    path[0] = Vertex::createCamera(&camera, ray, throughput);
    camera.pdf_We(ray, &pdfPos, &pdfDir);
    
    return randomWalk(scene, ray, sampler, arena,
                      throughput, pdfDir, maxDepth - 1,
                      TransportMode::Radiance, path + 1) + 1;
}

int generateLightSubpath(const Scene &scene, Sampler &sampler, MemoryArena &arena,
                        int maxDepth,Float time, const Distribution1D &lightDistr,
                        const std::unordered_map<const Light *, size_t> &lightToIndex,
                         Vertex *path) {
    if (maxDepth == 0)
        return 0;
    Float lightPdf;
    // 根据分布选择light
    int lightNum = lightDistr.sampleDiscrete(sampler.get1D(), &lightPdf);
    
    const std::shared_ptr<const Light> &light = scene.lights[lightNum];
    RayDifferential ray;
    Normal3f nLight;
    Float pdfPos, pdfDir;
    
    Spectrum Le = light->sample_Le(sampler.get2D(), sampler.get2D(),
                                time, &ray, &nLight, &pdfPos, &pdfDir);
    if (pdfPos == 0 || pdfDir == 0 || Le.IsBlack()) {
        return 0;
    }
    // 表示选中光源表面点的概率密度函数
    Float pdf = pdfPos * lightPdf;
    path[0] = Vertex::createLight(light.get(), ray, nLight, Le, pdf);
    
    Spectrum throughput = Le * absDot(nLight, ray.dir) / (pdf * pdfDir);
    
    int nVertices = randomWalk(scene, ray, sampler, arena,
                               throughput, pdfDir, maxDepth - 1,
                               TransportMode::Importance, path + 1);
    // 因为infiniteLight采样的范围是在
    // 如果采样的是环境光源，则光源路径第二个点
    // 否则用pdfDir
    if (path[0].isInfiniteLight()) {
        if (nVertices > 0) {
            path[1].pdfFwd = pdfPos;
            if (path[1].isOnSurface()) {
                path[1].pdfFwd *= absDot(path[0].ng(), ray.dir);
            }
        }
        path[0].pdfFwd = infiniteLightPdf(scene, lightDistr, lightToIndex, ray.dir);
    }
    
    return nVertices + 1;
}

int randomWalk(const Scene &scene, RayDifferential ray, Sampler &sampler,
                MemoryArena &arena, Spectrum throughput, Float pdf, int maxDepth,
               TransportMode mode, Vertex *path) {
    
    if (maxDepth == 0)
        return 0;
    int bounces = 0;
    Float pdfFwd = pdf, pdfRev = 0;
    // 基本思路
    // 根据指定的ray，按照衰减分布采样medium
    // 则根据采样到的点的p函数或者bsdf函数分布生成下一个顶点
    while (true) {
        
        MediumInteraction mi;
        SurfaceInteraction isect;
        
        bool foundIntr = scene.intersect(ray, &isect);
        
        if (ray.medium) {
            throughput *= ray.medium->sample(ray, sampler, arena, &mi);
        }
        if (throughput.IsBlack()) {
            break;
        }
        
        Vertex &vertex = path[bounces];
        Vertex &prev = path[bounces - 1];
        
        if (mi.isValid()) {
            Vector3f wo = -ray.dir;
            Vector3f wi;
            vertex = Vertex::createMedium(mi, throughput, pdfFwd, prev);
            if (++bounces >= maxDepth) {
                break;
            }
            pdfFwd = pdfRev = mi.phase->sample_p(wo, &wi, sampler.get2D());
            ray = mi.spawnRay(wi);
            
        } else {
            if (!foundIntr) {
                // 如果是从相机发出的射线，没有交点时，最后一个顶点是环境光源上的
                if (mode == TransportMode::Radiance) {
                    vertex = Vertex::createLight(EndpointInteraction(ray),
                                                 throughput, pdfFwd);
                    ++bounces;
                }
            }
            
            if (isect.bsdf == nullptr) {
                ray = isect.spawnRay(ray.dir);
                continue;
            }
            
            vertex = Vertex::createSurface(isect, throughput, pdfFwd, prev);
            if (++bounces >= maxDepth) {
                break;
            }
            Vector3f wo = isect.wo;
            Vector3f wi;
            BxDFType type;
            Spectrum f = isect.bsdf->sample_f(wo, &wi, sampler.get2D(), 
                                            &pdfFwd, BSDF_ALL, &type);
            if (f.IsBlack() || pdfFwd == 0.f) {
                break;
            }
            throughput *= f * absDot(wi, isect.shading.normal) / pdfFwd;
            pdfRev = isect.bsdf->pdfDir(wo, wi, BSDF_ALL);
            
            if (type & BSDF_SPECULAR) {
                vertex.delta = true;
                pdfRev = pdfFwd = 0;
            }
            throughput *= correctShadingNormal(isect, wo, wi, mode);
            ray = isect.spawnRay(wi);
        }
        prev.pdfRev = vertex.convertPdf(pdfRev, prev);
    }
    return bounces;
}

Spectrum connectPath(const Scene &scene, Vertex *lightVertices, 
                    Vertex *cameraVertices, int s, int t, 
                    const Distribution1D &lightDistr,
                    const std::unordered_map<const Light *, size_t> &lightToIndex,
                    const Camera &camera, Sampler &sampler, Point2f *pRaster,
                    Float *misWeight) {
    
    Spectrum L(0.f);
    // 如果相机路径最后一个顶点是环境光，则记作无效链接
    if (t > 1 && s != 0 && cameraVertices[t - 1].type == VertexType::Light) {
        return Spectrum(0.f);
    }
    
    Vertex sampled;
    if (s == 0) {
        // 判断相机路径是否直接击中光源
        const Vertex &pt = cameraVertices[t - 1];
        if (pt.isLight()) {
            // 将路径解释为一个完整路径
            L = pt.Le(scene, cameraVertices[t - 2]) * pt.throughput;
        }
        DCHECK(!L.HasNaNs());
    } else if (s == 1) {
        // 相机路径顶点直连光源
        const Vertex &pt = cameraVertices[t - 1];
        if (pt.isConnectible()) {
            Float lightPdf;
            VisibilityTester vis;
            Vector3f wi;
            Float pdfDir;
            int lightIdx = lightDistr.sampleDiscrete(sampler.get1D(), &lightPdf);
            const shared_ptr<Light> &light = scene.lights[lightIdx];
            Spectrum lightWeight = light->sample_Li(pt.getInteraction(), sampler.get2D(), &wi, &pdfDir, &vis);
            if (pdfDir > 0 && !lightWeight.IsBlack()) {
                EndpointInteraction ei(vis.P1(), light.get());
                sampled = Vertex::createLight(ei, lightWeight/(pdfDir * lightPdf), 0);
                sampled.pdfFwd = sampled.pdfLightOrigin(scene, pt, lightDistr, lightToIndex);
                L = pt.throughput * pt.f(sampled, TransportMode::Radiance) * sampled.throughput;
                if (pt.isOnSurface()) {
                    L *= absDot(pt.ns(), wi);
                }
                if (!L.IsBlack()) {
                    L *= vis.Tr(scene, sampler);
                }
                DCHECK(!L.HasNaNs());
            }
        }
    } else if (t == 1) {
        // 光源路径中的点直连相机
        const Vertex &qs = lightVertices[s - 1];
        if (qs.isConnectible()) {
            VisibilityTester vis;
            Vector3f wi;
            Float pdf;
            Spectrum Wi = camera.sample_Wi(qs.getInteraction(), sampler.get2D(), &wi, &pdf, pRaster, &vis);
            if (pdf > 0 && !Wi.IsBlack()) {
                sampled = Vertex::createCamera(&camera, vis.P0(), Wi / pdf);
                L = qs.throughput * qs.f(sampled, TransportMode::Importance) * sampled.throughput;
                if (qs.isOnSurface()) {
                    L *= absDot(qs.ns(), wi);
                }
                if (!L.IsBlack()) {
                    L *= vis.Tr(scene, sampler);
                }
                DCHECK(!L.HasNaNs());
            }
        }
    } else {
        const Vertex &qs = lightVertices[s - 1];
        const Vertex &pt = cameraVertices[t - 1];
        if (qs.isConnectible() && pt.isConnectible()) {
            L = qs.throughput * qs.f(pt, TransportMode::Importance)
                * pt.f(qs, TransportMode::Radiance) * pt.throughput;
            if (!L.IsBlack()) {
                L *= G(scene, sampler, qs, pt);
            }
            DCHECK(!L.HasNaNs());
        }
    }
    
    Float weight = L.IsBlack() ? 0.f : MISWeight(scene, lightVertices, cameraVertices,
                                            sampled, s, t, lightDistr, lightToIndex);
    DCHECK(!isNaN(weight));
    *misWeight = weight;
    L *= weight;
    
    return L;
}

Float MISWeight(const Scene &scene, Vertex *lightVertices,
                Vertex *cameraVertices, Vertex &sampled, int s, int t,
                const Distribution1D &lightPdf,
                const std::unordered_map<const Light *, size_t> &lightToIndex) {
    if (s + t == 2) {
        return 1;
    }
    Float sumRi = 0;
    // 将0重映射为1
    auto remap0 = [](Float f) -> Float { return f != 0 ? f : 1; };
    
    // 对于同一系列的顶点，不同的s,t组成了不同的连接策略，连接策略的改变
    // 可能导致前后PDF的改变
    Vertex * pt = t > 0 ? &cameraVertices[t - 1] : nullptr;
    Vertex * ptMinus = t > 1 ? &cameraVertices[t - 2] : nullptr;
    Vertex * qs = s > 0 ? &lightVertices[s - 1] : nullptr;
    Vertex * qsMinus = s > 1 ? &lightVertices[s - 2] : nullptr;
    
    ScopedAssignment<Vertex> a1;
    
    if (s == 1) {
        // 直连光源
        // 临时把光源顶点换成sampled顶点
        a1 = {qs, sampled};
    } else if (t == 1) {
        // 直连相机
        // 临时把相机顶点换成sampled顶点
        a1 = {pt, sampled};
    }
    
    // 如果pt与qs可以连接，说明这两个顶点都包含非specular分量
    // 在计算MIS时，是不能忽略的，所以要临时把delta改为false
    ScopedAssignment<bool> a2, a3;
    if (pt)
        a2 = {&pt->delta, false};
    if (qs)
        a3 = {&qs->delta, false};
    
    ScopedAssignment<Float> a4;
    if (pt) {
        // 当s=0时，pt落在光源上，pdfRev为采样到当前光源的pdf
        // 当s>0时，为qs采样到pt的PDF
        a4 = {&pt->pdfRev, s > 0
            ? qs->pdfDir(scene, qsMinus, *pt)
            : pt->pdfLightOrigin(scene, *ptMinus, lightPdf,
                             lightToIndex)};
    }
    
    ScopedAssignment<Float> a5;
    if (ptMinus) {
        // 当s=0时，pt落在光源上
        a5 = {&ptMinus->pdfRev, s > 0
            ? pt->pdfDir(scene, qs, *ptMinus)
            : pt->pdfLight(scene, *ptMinus)};
    }
    
    ScopedAssignment<Float> a6;
    if (qs) {
        a6 = {&qs->pdfRev, pt->pdfDir(scene, ptMinus, *qs)};
    }
    ScopedAssignment<Float> a7;
    if (qsMinus) {
        a7 = {&qsMinus->pdfRev, qs->pdfDir(scene, pt, *qsMinus)};
    }
    
    // 先把ri赋值为rs的值，为1
    Float ri = 1;
    for (int i = s - 1; i >= 0; --i) {
        auto v = lightVertices[i];
        ri = ri * (remap0(v.pdfRev) / remap0(v.pdfFwd));
        bool deltaLightvertex = i > 0 ? lightVertices[i - 1].delta
                                : lightVertices[0].isDeltaLight();
        
        if (!deltaLightvertex && !v.delta) {
            sumRi += ri;
        }
    }
    // 先把ri赋值为rs的值，为1
    ri = 1;
    for (int i = t; i > 0; --i) {
        auto v = cameraVertices[i];
        ri = ri * (remap0(v.pdfRev) / remap0(v.pdfFwd));
        if (!cameraVertices[i].delta && !cameraVertices[i - 1].delta) {
            sumRi += ri;
        }
    }
    return 1 / (sumRi + 1);
}

PALADIN_END
