//
//  lightdistribute.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/30.
//

#include "lightdistribute.hpp"
#include "core/scene.hpp"

PALADIN_BEGIN

std::unique_ptr<Distribution1D> computeLightPowerDistribution(const Scene &scene) {
    if (scene.lights.empty()) {
        return nullptr;
    }
    std::vector<Float> lightPower;
    for (const auto &light : scene.lights) {
        lightPower.push_back(light->power().y());
    }
    return std::unique_ptr<Distribution1D>(new Distribution1D(&lightPower[0], lightPower.size()));
}

UniformLightDistribution::UniformLightDistribution(const Scene &scene) {
    std::vector<Float> prob(scene.lights.size(), Float(1));
    _distribute.reset(new Distribution1D(&prob[0], int(prob.size())));
}

const Distribution1D * UniformLightDistribution::lookup(const Point3f &p) const {
    return _distribute.get();
}

PowerLightDistribution::PowerLightDistribution(const Scene &scene)
: _distribute(computeLightPowerDistribution(scene)) {
    
}

const Distribution1D *PowerLightDistribution::lookup(const Point3f &p) const {
    return _distribute.get();
}

PALADIN_END
