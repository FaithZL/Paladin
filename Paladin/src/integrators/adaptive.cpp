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
            
        };
    }
}


PALADIN_END
