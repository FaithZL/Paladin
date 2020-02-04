//
//  bidirectional.h
//  Paladin
//
//  Created by SATAN_Z on 2020/2/3.
//

#ifndef bidirectional_h
#define bidirectional_h

#include "vertex.h"
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

extern Spectrum connectBDPT(const Scene &scene, Vertex *lightVertices, Vertex *cameraVertices, int s,
                    int t, const Distribution1D &lightDistr,
                    const std::unordered_map<const Light *, size_t> &lightToIndex,
                    const Camera &camera, Sampler &sampler, Point2f *pRaster,
                    Float *misWeight = nullptr);

PALADIN_END


#endif /* bidirectional_h */
