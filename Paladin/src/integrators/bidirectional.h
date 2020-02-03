//
//  bidirectional.h
//  Paladin
//
//  Created by SATAN_Z on 2020/2/3.
//

#ifndef bidirectional_h
#define bidirectional_h

#include "core/header.h"

PALADIN_BEGIN

// 双向方法y通用的一些函数

extern int generateCameraSubpath(const Scene &scene, Sampler &sampler,
                                 MemoryArena &arena, int maxDepth,
                                 const Camera &camera, const Point2f &pFilm,
                                 Vertex *path);

extern int generateLightSubpath(const Scene &scene, Sampler &sampler, MemoryArena &arena,
                                int maxDepth,Float time, const Distribution1D &lightDistr,
                                const std::unordered_map<const Light *, size_t> &lightToIndex,
                                Vertex *path);

Spectrum connectBDPT(const Scene &scene, Vertex *lightVertices, Vertex *cameraVertices, int s,
                    int t, const Distribution1D &lightDistr,
                    const std::unordered_map<const Light *, size_t> &lightToIndex,
                    const Camera &camera, Sampler &sampler, Point2f *pRaster,
                    Float *misWeight = nullptr);

PALADIN_END


#endif /* bidirectional_h */
