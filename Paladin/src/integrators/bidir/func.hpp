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

/**
 * 随机生成路径顶点
 * @param  scene    场景
 * @param  ray      
 * @param  sampler  
 * @param  arena    
 * @param  beta     吞吐量
 * @param  pdf      选中当前起点的PDF
 * @param  maxDepth 最大深度
 * @param  mode     传输模式
 * @param  path     路径
 * @return          返回顶点数量
 */
int randomWalk(const Scene &scene, RayDifferential ray, Sampler &sampler,
                MemoryArena &arena, Spectrum throughput, Float pdf, int maxDepth,
                TransportMode mode, Vertex *path, Float rrThreshold = 1);

int generateCameraSubpath(const Scene &scene, Sampler &sampler,
                                 MemoryArena &arena, int maxDepth,
                                 const Camera &camera, const Point2f &pFilm,
                                 Vertex *path, Float rrThreshold = 1);

int generateLightSubpath(const Scene &scene, Sampler &sampler, MemoryArena &arena,
                                int maxDepth,Float time, const Distribution1D &lightDistr,
                                const std::unordered_map<const Light *, size_t> &lightToIndex,
                                Vertex *path, Float rrThreshold = 1);

// 推导过程详见bdpt.hpp文件中
Float MISWeight(const Scene &scene, Vertex *lightVertices,
                Vertex *cameraVertices, Vertex &sampled, int s, int t,
                const Distribution1D &lightPdf,
                const std::unordered_map<const Light *, size_t> &lightToIndex);

Spectrum connectPath(const Scene &scene, Vertex *lightVertices, 
                    Vertex *cameraVertices, int s, int t, 
                    const Distribution1D &lightDistr,
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

/**
 * 采样光源的pdf，基于面积的PDF
 * @param  scene             [description]
 * @param  lightDistr        [description]
 * @param  lightToDistrIndex [description]
 * @param  w                 [description]
 * @return                   [description]
 */
Float infiniteLightPdf(
    const Scene &scene, const Distribution1D &lightDistr,
    const std::unordered_map<const Light *, size_t> &lightToDistrIndex,
                                  const Vector3f &w);

Spectrum G(const Scene &scene, Sampler &sampler, 
            const Vertex &v0,const Vertex &v1);

PALADIN_END

#endif /* func_hpp */
