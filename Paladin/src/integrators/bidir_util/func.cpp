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

PALADIN_BEGIN

Float infiniteLightDensity(
    const Scene &scene, const Distribution1D &lightDistr,
    const std::unordered_map<const Light *, size_t> &lightToDistrIndex,
    const Vector3f &w) {
    Float pdf = 0;
    for (const auto &light : scene.infiniteLights) {
        CHECK(lightToDistrIndex.find(light.get()) != lightToDistrIndex.end());
        size_t index = lightToDistrIndex.find(light.get())->second;
        pdf += light->pdf_Li(Interaction(), -w) * lightDistr.funcAt(index);
    }
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
                      TransportMode::Importance, path + 1) + 1;
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
    
    Spectrum Le = light->sample_Le(sampler.get2D(), sampler.get2D(), time, &ray, &nLight, &pdfPos, &pdfDir);
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
    
    return nVertices + 1;
}

int randomWalk(const Scene &scene, RayDifferential ray, Sampler &sampler,
                MemoryArena &arena, Spectrum throughput, Float pdf, int maxDepth,
               TransportMode mode, Vertex *path) {
    
    if (maxDepth == 0)
        return 0;
    int bounces = 0;
    Float pdfFwd = pdf, pdfRev = 0;
    
}

PALADIN_END
