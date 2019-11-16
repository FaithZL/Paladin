//
//  random.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "random.hpp"

PALADIN_BEGIN

Float RandomSampler::get1D() {
    CHECK_LT(_currentPixelSampleIndex, samplesPerPixel);
    return _rng.uniformFloat();
}

Point2f RandomSampler::get2D() {
    CHECK_LT(_currentPixelSampleIndex, samplesPerPixel);
    return Point2f(_rng.uniformFloat(), _rng.uniformFloat());
}

std::unique_ptr<Sampler> RandomSampler::clone(int seed) {
    RandomSampler *rs = new RandomSampler(*this);
    rs->_rng.setSequence(seed);
    return std::unique_ptr<Sampler>(rs);
}

void RandomSampler::startPixel(const Point2i &p) {
    for (size_t i = 0; i < _sampleArray1D.size(); ++i)
        for (size_t j = 0; j < _sampleArray1D[i].size(); ++j)
            _sampleArray1D[i][j] = _rng.uniformFloat();
    
    for (size_t i = 0; i < _sampleArray2D.size(); ++i)
        for (size_t j = 0; j < _sampleArray2D[i].size(); ++j)
            _sampleArray2D[i][j] = {_rng.uniformFloat(), _rng.uniformFloat()};
    Sampler::startPixel(p);
}

neb::CJsonObject RandomSampler::toJson() const {
    return nebJson();
}

/**
 * param : {
 *     "spp" : 8
 * }
 */
CObject_ptr createRandomSampler(const nebJson &param) {
    int spp = param.GetValue("spp", 8);
    return make_shared<RandomSampler>(spp);
}

REGISTER("random", createRandomSampler);

PALADIN_END
