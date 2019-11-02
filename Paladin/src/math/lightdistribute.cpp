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
    _distribution.reset(new Distribution1D(&prob[0], int(prob.size())));
}

const Distribution1D * UniformLightDistribution::lookup(const Point3f &p) const {
    return _distribution.get();
}

PowerLightDistribution::PowerLightDistribution(const Scene &scene)
: _distribution(computeLightPowerDistribution(scene)) {
    
}

const Distribution1D *PowerLightDistribution::lookup(const Point3f &p) const {
    return _distribution.get();
}

std::unique_ptr<LightDistribution> createLightSampleDistribution(
    const std::string &name, const Scene &scene) {
    if (name == "uniform" || scene.lights.size() == 1)
        return std::unique_ptr<LightDistribution>{
            new UniformLightDistribution(scene)};
    else if (name == "power")
        return std::unique_ptr<LightDistribution>{
            new PowerLightDistribution(scene)};
    else {
        COUT << (
            "Light sample distribution type \"%s\" unknown. Using \"power\".",
            name.c_str());
        return std::unique_ptr<LightDistribution>{
            new PowerLightDistribution(scene)};
    }
}

PALADIN_END
