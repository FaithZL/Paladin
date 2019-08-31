//
//  sampler.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "sampler.hpp"
#include "camera.hpp"

PALADIN_BEGIN

Sampler::Sampler(int64_t samplesPerPixel)
: samplesPerPixel(samplesPerPixel) {
    
}

void Sampler::startPixel(const Point2i &p) {
    _currentPixel = p;
    _currentPixelSampleIndex = 0;
    _array1DOffset = _array2DOffset = 0;
}

CameraSample Sampler::getCameraSample(const Point2i &pRaster) {
    CameraSample ret;
    ret.pFilm = (Point2f)pRaster + get2D();
    ret.pLens = get2D();
    ret.time = get1D();
    return ret;
}

void Sampler::request1DArray(int n) {
    CHECK_EQ(n, roundCount(n));
    _samples1DArraySizes.push_back(n);
    _sampleArray1D.push_back(std::vector<Float>(n * samplesPerPixel));
}

void Sampler::request2DArray(int n) {
    CHECK_EQ(n, roundCount(n));
    _samples2DArraySizes.push_back(n);
    _sampleArray2D.push_back(std::vector<Point2f>(n * samplesPerPixel));
}

const Float * Sampler::get1DArray(int n) {
    if (_array1DOffset == _sampleArray2D.size()) {
        return nullptr;
    }
    CHECK_EQ(_samples1DArraySizes[_array1DOffset], n);
    CHECK_LT(_currentPixelSampleIndex, samplesPerPixel);
    return &(_sampleArray1D[_array1DOffset++][_currentPixelSampleIndex * n]);
}

const Point2f * Sampler::get2DArray(int n) {
    if (_array2DOffset == _sampleArray2D.size()) {
        return nullptr;
    }
    CHECK_EQ(_samples2DArraySizes[_array2DOffset], n);
    CHECK_LT(_currentPixelSampleIndex, samplesPerPixel);
    return &(_sampleArray2D[_array2DOffset++][_currentPixelSampleIndex * n]);
}

bool Sampler::startNextSample() {
    _array1DOffset = _array2DOffset = 0;
    return ++_currentPixelSampleIndex < samplesPerPixel;
}

bool Sampler::setSampleNumber(int64_t sampleNum) {
    _array1DOffset = _array2DOffset = 0;
    _currentPixelSampleIndex = sampleNum;
    return _currentPixelSampleIndex < samplesPerPixel;
}

PALADIN_END





