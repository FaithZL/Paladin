//
//  func.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/2/7.
//

#ifndef func_hpp
#define func_hpp

#include "vertex.hpp"
#include "assignment.h"

PALADIN_BEGIN

// 双向方法y通用的一些函数

int randomWalk(const Scene &scene, RayDifferential ray, Sampler &sampler,
                MemoryArena &arena, Spectrum beta, Float pdf, int maxDepth,
                TransportMode mode, Vertex *path);

extern int generateCameraSubpath(const Scene &scene, Sampler &sampler,
                                 MemoryArena &arena, int maxDepth,
                                 const Camera &camera, const Point2f &pFilm,
                                 Vertex *path);

extern int generateLightSubpath(const Scene &scene, Sampler &sampler, MemoryArena &arena,
                                int maxDepth,Float time, const Distribution1D &lightDistr,
                                const std::unordered_map<const Light *, size_t> &lightToIndex,
                                Vertex *path);

Float MISWeight(const Scene &scene, Vertex *lightVertices,
                Vertex *cameraVertices, Vertex &sampled, int s, int t,
                const Distribution1D &lightPdf,
                const std::unordered_map<const Light *, size_t> &lightToIndex);

extern Spectrum connectPath(const Scene &scene, Vertex *lightVertices, Vertex *cameraVertices, int s,
                    int t, const Distribution1D &lightDistr,
                    const std::unordered_map<const Light *, size_t> &lightToIndex,
                    const Camera &camera, Sampler &sampler, Point2f *pRaster,
                    Float *misWeight = nullptr);

/**
 *                              wo · Ns
 * f(wo->wi) = f_Ns(wo->wi) |-------------| = f_adjoint(wi->wo)
 *                              wo · Ng
 *
 * 假设 f_Ns(wi->wo) = f_Ns(wo->wi)，则
 * 
 *  f_adjoint(wi->wo)     wo · Ns     wi · Ng
 * ------------------ = |---------| |---------|
 *     f(wi->wo)          wo · Ng     wi · Ns
 *
 * 给每个f函数添加一个校正因数(correct factor cf)
 *
 *        wo · Ns     wi · Ng
 * cf = |---------| |---------|  (importance transport)
 *        wo · Ng     wi · Ns
 *
 * cf = 1 (radiance transport)
 * 具体推导详见bdpt.hpp
 * Eric veach 博士的论文有详细推导
 * @param  isect [description]
 * @param  wo    [description]
 * @param  wi    [description]
 * @param  mode  [description]
 * @return       [description]
 */
Float correctShadingNormal(const SurfaceInteraction &isect, const Vector3f &wo,
                           const Vector3f &wi, TransportMode mode);

Float infiniteLightDensity(
    const Scene &scene, const Distribution1D &lightDistr,
    const std::unordered_map<const Light *, size_t> &lightToDistrIndex,
                                  const Vector3f &w);

Spectrum G(const Scene &scene, Sampler &sampler, const Vertex &v0,
           const Vertex &v1);

PALADIN_END

#endif /* func_hpp */
