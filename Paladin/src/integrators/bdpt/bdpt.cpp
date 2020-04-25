//
//  bdpt.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/12/31.
//

#include "bdpt.hpp"
#include "math/lightdistribute.hpp"
#include "tools/progressreporter.hpp"
#include "../bidir/func.hpp"


PALADIN_BEGIN

void BidirectionalPathTracer::render(const Scene &scene) {
    std::unique_ptr<LightDistribution> lightDistribution = createLightSampleDistribution(_lightSampleStrategy, scene);
    
    std::unordered_map<const Light *, size_t> lightToIndex;
    for (size_t i = 0; i < scene.lights.size(); ++i) {
        lightToIndex[scene.lights[i].get()] = i;
    }
    
    Film *film = _camera->film.get();
    const AABB2i sampleBounds = film->getSampleBounds();
    const Vector2i sampleExtent = sampleBounds.diagonal();
    
    const int tileSize = 16;
    const int nXTiles = (sampleExtent.x + tileSize - 1) / tileSize;
    const int nYTiles = (sampleExtent.y + tileSize - 1) / tileSize;
    
    ProgressReporter reporter("rendering", nXTiles * nYTiles);
    
    if (scene.lights.size() > 0) {
        auto renderTile = [&](Point2i tile, int threadIdx) {
            MemoryArena arena;
            int seed = tile.y * nXTiles + tile.x;
            std::unique_ptr<Sampler> tileSampler = _sampler->clone(seed);
            int x0 = sampleBounds.pMin.x + tile.x * tileSize;
            int x1 = std::min(x0 + tileSize, sampleBounds.pMax.x);
            int y0 = sampleBounds.pMin.y + tile.y * tileSize;
            int y1 = std::min(y0 + tileSize, sampleBounds.pMax.y);
            AABB2i tileBounds(Point2i(x0, y0), Point2i(x1, y1));
            
            std::unique_ptr<FilmTile> filmTile = _camera->film->getFilmTile(tileBounds);
            
            for (Point2i pPixel : tileBounds) {
                tileSampler->startPixel(pPixel);
                if (!insideExclusive(pPixel, _pixelBounds)) {
                    continue;
                }
                
                do {
                    
                    Point2f pFilm = (Point2f)pPixel + tileSampler->get2D();
                    
                    // 相机路径可能会直接击中光源
                    Vertex *cameraVertices = arena.alloc<Vertex>(_maxDepth + 2);
                    Vertex *lightVertices = arena.alloc<Vertex>(_maxDepth + 1);
                    
                    int nCamera = generateCameraSubpath(scene, *tileSampler, arena, _maxDepth + 2,
                                                        *_camera, pFilm, cameraVertices, _rrThreshold);
                    // 生成光源分布，因为ray可能会反弹很多次
                    // 所以任何一个顶点的spatial光源分布都未必是好的分布
                    // 所以在这里我们默认使用power分布
                    const Distribution1D *lightDistr =
                    lightDistribution->lookup(cameraVertices[0].pos());
                    
                    int nLight = generateLightSubpath(scene, *tileSampler, arena, _maxDepth + 1,
                                                    cameraVertices[0].time(), *lightDistr,
                                                    lightToIndex, lightVertices, _rrThreshold);
                    
                    // 遍历相机光源顶点列表，逐个策略尝试连接计算贡献
                    Spectrum L(0.f);
                    for (int t = 1; t <= nCamera; ++t) {
                        for (int s = 0; s <= nLight; ++s) {
                            int depth = t + s - 2;
                            // Camera::sample_Wi()与Light::sample_Li() 不能同时使用
                            // 所以忽略s=t=1的情况，从相机出射直接击中光源的情况由t = 1处理
                            if ((s == 1 && t == 1)
                                || depth < 0
                                || depth > _maxDepth) {
                                continue;
                            }
                            
                            Point2f pFilmNew = pFilm;
                            Float misWeight = 0.f;
                            // 渲染方程转化为路径形式
                            // 可以理解为Le加上一个路径长度为2的一重积分加上路径长度为3的二重积分
                            // 加上路径长度为4的三重积分。。。。。等 n 个积分方程的和
                            // Lpath的理解如下
                            // 假设一条路径的顶点数量为5，那么有5种采样策略可以构成该路径
                            // Lpath就可以理解为其中一种采样策略经过mis计算之后是值，
                            // balance heuristic需要知道同一条路径的其他策略的pdf，从而计算mis
                            // 5种策略的Lpath求和就是对这个5重积分的估计值
                            // connectPath根据传入的s,t来决定路径的长度
                            // 内部再根据同一列顶点的不同采样策略进行mis的估计
                            // 所谓不同采样策略就是5个相机顶点与6个光源顶点，或者6个相机顶点与5个光源顶点
                            Spectrum Lpath = connectPath(scene, lightVertices, cameraVertices,
                                                         s, t, *lightDistr, lightToIndex,
                                                         *_camera, *tileSampler,
                                                         &pFilmNew, &misWeight);
                            
                            if (t != 1) {
                                L += Lpath;
                            } else {
                                film->addSplat(pFilmNew, Lpath);
                            }
                        }
                    }
                    filmTile->addSample(pFilm, L);
                    arena.reset();
                    
                } while (tileSampler->startNextSample());
            }
            film->mergeFilmTile(std::move(filmTile));
            reporter.update();
        };
        parallelFor2D(renderTile, Point2i(nXTiles, nYTiles));
    }
    film->writeImage(1.f / _sampler->samplesPerPixel);
}

//"param" : {
//    "maxBounce" : 5,
//    "rrThreshold" : 1,
//    "strategies" : false,
//    "weights" : false,
//    "lightSampleStrategy" : "power"
//}
// lst = {sampler, camera}
CObject_ptr createBDPT(const nloJson &param, const Arguments &lst) {
    int maxBounce = param.value("maxBounce", 5);
    Float rrThreshold = param.value("rrThreshold", 1.f);
    string lightSampleStrategy = param.value("lightSampleStrategy", "power");
    bool showStrategies = param.value("strategies", false);
    bool showWeight = param.value("weights", false);
    auto iter = lst.begin();
    Sampler * sampler = dynamic_cast<Sampler *>(*iter);
    ++iter;
    Camera * camera = dynamic_cast<Camera *>(*iter);
    AABB2i pixelBounds = camera->film->getSampleBounds();
    
    return new BidirectionalPathTracer(shared_ptr<Sampler>(sampler),
                                       shared_ptr<const Camera>(camera),
                                       maxBounce,
                                       showStrategies,
                                       showWeight,
                                       pixelBounds,
                                       rrThreshold,
                                       lightSampleStrategy);
}

REGISTER("bdpt", createBDPT);

PALADIN_END
