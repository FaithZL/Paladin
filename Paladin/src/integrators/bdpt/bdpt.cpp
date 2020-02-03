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
    
    
}

PALADIN_END
