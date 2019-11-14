//
//  stratified.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/9/3.
//

#include "stratified.hpp"
#include "math/sampling.hpp"

PALADIN_BEGIN

void StratifiedSampler::startPixel(const Point2i &p) {
    // 为每个像素生成一系列单独的样本，然后乱序
    size_t count = _xPixelSamples * _xPixelSamples;
    for (size_t i = 0; i < _samples1D.size(); ++i) {
        stratifiedSample1D(&_samples1D[i][0], count, _rng, _jitterSamples);
        // 下标i代表维度
        // 重排的目的是为了避免不同维度之间对应同一个cell，形成artifact
        shuffle(&_samples1D[i][0], count, 1, _rng);
    }
    for (size_t i = 0; i < _samples2D.size(); ++i) {
        stratifiedSample2D(&_samples2D[i][0], _xPixelSamples, _yPixelSamples, _rng, _jitterSamples);
        shuffle(&_samples2D[i][0], count, 1, _rng);
    }
    
    for (size_t i = 0; i < _sampleArray1D.size(); ++i) {
        for (size_t j = 0; j < samplesPerPixel; ++j) {
            size_t count = _samples1DArraySizes[i];
            stratifiedSample1D(&_sampleArray1D[i][j * count], count, _rng, _jitterSamples);
            shuffle(&_sampleArray1D[i][j * count], count, 1, _rng);
        }
    }
    for (size_t i = 0; i < _sampleArray2D.size(); ++i) {
        for (size_t j = 0; j < samplesPerPixel; ++j) {
            size_t count = _samples2DArraySizes[i];
            latinHypercube(&_sampleArray2D[i][j * count].x, count, 2, _rng);
        }
    }
    PixelSampler::startPixel(p);
}

neb::CJsonObject StratifiedSampler::toJson() const {
    
}

std::unique_ptr<Sampler> StratifiedSampler::clone(int seed) {
    StratifiedSampler * ret = new StratifiedSampler(*this);
    ret->_rng.setSequence(seed);
    return std::unique_ptr<Sampler>(ret);
}

Serialize_ptr createStratifiedSampler(const nebJson &param) {
    bool jitter = params.getValue("jitter", true);
    int xsamp = params.getValue("xsamples", 4);
    int ysamp = params.getValue("ysamples", 4);
    int sd = params.getValue("dimensions", 4);
    Serialize_ptr ret = make_shared<StratifiedSampler>(xsamp, ysamp, jitter, sd);
    return ret;
}

REGISTER("stratified", createStratifiedSampler);

PALADIN_END
