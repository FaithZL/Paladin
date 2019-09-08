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

bool Sampler::setSampleIndex(int64_t sampleNum) {
    _array1DOffset = _array2DOffset = 0;
    _currentPixelSampleIndex = sampleNum;
    return _currentPixelSampleIndex < samplesPerPixel;
}

// PixelSampler
PixelSampler::PixelSampler(int64_t samplesPerPixel, int nSampledDimensions)
: Sampler(samplesPerPixel) {
    for (int i = 0; i < nSampledDimensions; ++i) {
        _samples1D.push_back(std::vector<Float>(samplesPerPixel));
        _samples2D.push_back(std::vector<Point2f>(samplesPerPixel));
    }
}

bool PixelSampler::startNextSample() {
    _curDimension1D = _curDimension2D = 0;
    return Sampler::startNextSample();
}

bool PixelSampler::setSampleIndex(int64_t sampleNum) {
    _curDimension1D = _curDimension2D = 0;
    return Sampler::setSampleIndex(sampleNum);
}

Float PixelSampler::get1D() {
    CHECK_LT(_currentPixelSampleIndex, samplesPerPixel);
    if (_curDimension1D < _samples1D.size()) {
        return _samples1D[_curDimension1D++][_currentPixelSampleIndex];
    } else {
        return _rng.uniformFloat();
    }
}

Point2f PixelSampler::get2D() {
    CHECK_LT(_currentPixelSampleIndex, samplesPerPixel);
    if (_curDimension2D < _samples2D.size()) {
        return _samples2D[_curDimension2D++][_currentPixelSampleIndex];
    } else {
        return Point2f(_rng.uniformFloat(), _rng.uniformFloat());
    }
}

// GlobalSampler
void GlobalSampler::startPixel(const Point2i &p) {
    Sampler::startPixel(p);
    _dimension = 0;
    _globalIndex = getIndexForSample(0);
    _arrayEndDim = _arrayStartDim + _sampleArray1D.size() + 2 * _sampleArray2D.size();
    
    // 生成一维样本数组
    for (size_t i = 0; i < _samples1DArraySizes.size(); ++i) {
        int nSamples = _samples1DArraySizes[i] * samplesPerPixel;
        for (int j = 0; j < nSamples; ++j) {
            int64_t index = getIndexForSample(j);
            _sampleArray1D[i][j] = sampleDimension(index, _arrayStartDim + i);
        }
    }
    
    // 生成二维样本数组
    int dim = _arrayStartDim + _samples1DArraySizes.size();
    for (size_t i = 0; i < _samples2DArraySizes.size(); ++i) {
        int nSamples = _samples2DArraySizes[i] * samplesPerPixel;
        for (int j = 0; j < nSamples; ++j) {
            int64_t idx = getIndexForSample(j);
            _sampleArray2D[i][j].x = sampleDimension(idx, dim);
            _sampleArray2D[i][j].y = sampleDimension(idx, dim + 1);
        }
        dim += 2;
    }
    CHECK_EQ(_arrayEndDim, dim);
}

bool GlobalSampler::startNextSample() {
    _dimension = 0;
    _globalIndex = getIndexForSample(_currentPixelSampleIndex + 1);
    return Sampler::startNextSample();
}

bool GlobalSampler::setSampleIndex(int64_t sampleNum) {
    _dimension = 0;
    _globalIndex = getIndexForSample(sampleNum);
    return Sampler::setSampleIndex(sampleNum);
}

Float GlobalSampler::get1D() {
    if (_dimension >= _arrayStartDim && _dimension < _arrayEndDim) {
        _dimension = _arrayEndDim;
    }
    return sampleDimension(_globalIndex, _dimension++);
}

Point2f GlobalSampler::get2D() {
    if (_dimension + 1 >= _arrayStartDim && _dimension < _arrayEndDim) {
        _dimension = _arrayEndDim;
    }
    Point2f p(sampleDimension(_globalIndex, _dimension),
              sampleDimension(_globalIndex, _dimension + 1));
    _dimension += 2;
    return p;
}

PALADIN_END










