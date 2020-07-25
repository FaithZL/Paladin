//
//  adaptive.cpp
//  Paladin
//
//  Created by Zero on 2020/7/19.
//

#include "adaptive.hpp"
#include "tools/progressreporter.hpp"
#include "core/camera.hpp"

PALADIN_BEGIN

STAT_COUNTER("Integrator/Camera rays traced", nCameraRays);

void AdaptiveIntegrator::preprocess(const Scene &scene, Sampler &sampler) {
    
}

void AdaptiveIntegrator::render(const Scene &scene) {
    TRY_PROFILE(Prof::IntegratorRender)
    _subIntegrator->preprocess(scene, *_sampler);
    
    // 由于是并行计算，先把屏幕分割成m * n块
    AABB2i samplerBounds = _camera->film->getSampleBounds();
    Vector2i sampleExtent = samplerBounds.diagonal();
    const int tileSize = 16;

    // 把屏幕x方向分为nTile.x部分，y方向分为nTile.y部分
    Point2i nTile((sampleExtent.x + tileSize - 1) / tileSize,
                (sampleExtent.y + tileSize - 1) / tileSize);
    
    ProgressReporter reporter("rendering", nTile.x * nTile.y);
    {
        auto renderTile = [&](Point2i tile, int threadIdx) {
            // 内存池对象，预先申请一大段连续内存
            // 之后所有内存全都通过arena分配
            MemoryArena arena;
            // 每个tile使用不同的随机种子，避免关联采样导致的artifact
            int seed = tile.y + nTile.x + tile.x;
            
            std::unique_ptr<Sampler> tileSampler = _sampler->clone(seed);

            // 计算当前tile的起始点与结束点
            int x0 = samplerBounds.pMin.x + tile.x * tileSize;
            int x1 = std::min(x0 + tileSize, samplerBounds.pMax.x);
            int y0 = samplerBounds.pMin.y + tile.y * tileSize;
            int y1 = std::min(y0 + tileSize, samplerBounds.pMax.y);
            AABB2i tileBounds(Point2i(x0, y0), Point2i(x1, y1));

            std::unique_ptr<FilmTile> filmTile = _camera->film->getFilmTile(tileBounds);
            Float diffScale = 1 / std::sqrt((Float)tileSampler->samplesPerPixel);
            
            for (Point2i pixel : tileBounds) {
                {
                    TRY_PROFILE(Prof::StartPixel)
                    tileSampler->startPixel(pixel);
                }
                if (!insideExclusive(pixel, _pixelBounds)) {
                    continue;
                }
                
                do {
                    // 循环单个像素，采样spp次
                    CameraSample cameraSample = tileSampler->getCameraSample(pixel);
                    
                    RayDifferential ray;
                    Float rayWeight = _camera->generateRayDifferential(cameraSample, &ray);
                    ++nCameraRays;
                    ray.scaleDifferentials(diffScale);
                    
                    Spectrum L(0.0f);
                    if (rayWeight > 0) {
                        L = Li(ray, scene, *tileSampler, arena);
                    }
                    
                    if (L.HasNaNs()) {
                        COUT << StringPrintf(
                                "Not-a-number radiance value returned "
                                "for pixel (%d, %d), sample %d. Setting to black.",
                                pixel.x, pixel.y,
                                (int)tileSampler->currentSampleIndex());
                        DCHECK(false);
                        L = Spectrum(0.0f);
                    } else if (L.y() < -1e-5) {
                        COUT << StringPrintf(
                                "Negative luminance value, %f, returned "
                                "for pixel (%d, %d), sample %d. Setting to black.",
                                L.y(), pixel.x, pixel.y,
                                (int)tileSampler->currentSampleIndex());
                        DCHECK(false);
                        L = Spectrum(0.0f);
                    } else if (std::isinf(L.y())) {
                        COUT << StringPrintf(
                                "Infinite luminance value returned "
                                "for pixel (%d, %d), sample %d. Setting to black.",
                                pixel.x, pixel.y,
                                (int)tileSampler->currentSampleIndex());
                        DCHECK(false);
                        L = Spectrum(0.0f);
                    }
                    
                    // 将像素样本值与权重保存到pixel像素数据中
                    filmTile->addSample(cameraSample.pFilm, L, rayWeight);
                    arena.reset();
                    
                } while (isContinue(tileSampler.get()));
            }
        };
    }
}

bool AdaptiveIntegrator::isContinue(Sampler * sampler) const {
    return true;
}



//"param" : {
//    "maxBounce" : 5,
//    "rrThreshold" : 1,
//    "lightSampleStrategy" : "power"
//}
// lst = {sampler, camera}
CObject_ptr createAdaptiveIntegrator(const nloJson &param, const Arguments &lst) {
    
    
}

REGISTER("adaptive", createAdaptiveIntegrator);

PALADIN_END
