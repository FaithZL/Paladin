//
//  bdpt.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/12/31.
//

#include "bdpt.hpp"
#include "math/lightdistribute.hpp"


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
}

PALADIN_END
