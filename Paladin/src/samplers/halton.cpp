//
//  halton.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/8/29.
//

#include "halton.hpp"

PALADIN_BEGIN

static CONSTEXPR int kMaxResolution = 128;

static void extendedGCD(uint64_t a, uint64_t b, int64_t *x, int64_t *y) {
    if (b == 0) {
        *x = 1;
        *y = 0;
        return;
    }
    int64_t d = a / b, xp, yp;
    extendedGCD(b, a % b, &xp, &yp);
    *x = yp;
    *y = xp - (d * yp);
}

static uint64_t multiplicativeInverse(int64_t a, int64_t n) {
    int64_t x, y;
    extendedGCD(a, n, &x, &y);
    return Mod(x, n);
}

HaltonSampler::HaltonSampler(int samplesPerPixel, const AABB2i &sampleBounds,
                             bool sampleAtPixelCenter)
: GlobalSampler(samplesPerPixel),
_sampleAtPixelCenter(sampleAtPixelCenter) {

    if (_radicalInversePermutations.empty()) {
        RNG rng;
        _radicalInversePermutations = ComputeRadicalInversePermutations(rng);
    }
    
    Vector2i res = sampleBounds.pMax - sampleBounds.pMin;
    for (int i = 0; i < 2; ++i) {
        int base = (i == 0) ? 2 : 3;
        int scale = 1, exp = 0;
        while (scale < std::min(res[i], kMaxResolution)) {
            scale *= base;
            ++exp;
        }
        _baseScales[i] = scale;
        _baseExponents[i] = exp;
    }

    _sampleStride = _baseScales[0] * _baseScales[1];
    
    _multInverse[0] = multiplicativeInverse(_baseScales[1], _baseScales[0]);
    _multInverse[1] = multiplicativeInverse(_baseScales[0], _baseScales[1]);
}

std::vector<uint16_t> HaltonSampler::_radicalInversePermutations;
int64_t HaltonSampler::getIndexForSample(int64_t sampleNum) const {
    if (_currentPixel != _pixelForOffset) {
        _offsetForCurrentPixel = 0;
        if (_sampleStride > 1) {
            Point2i pm(Mod(_currentPixel[0], kMaxResolution),
                       Mod(_currentPixel[1], kMaxResolution));
            for (int i = 0; i < 2; ++i) {
                uint64_t dimOffset =
                (i == 0)
                ? InverseRadicalInverse<2>(pm[i], _baseExponents[i])
                : InverseRadicalInverse<3>(pm[i], _baseExponents[i]);
                _offsetForCurrentPixel +=
                dimOffset * (_sampleStride / _baseScales[i]) * _multInverse[i];
            }
            _offsetForCurrentPixel %= _sampleStride;
        }
        _pixelForOffset = _currentPixel;
    }
    return _offsetForCurrentPixel + sampleNum * _sampleStride;
}

Float HaltonSampler::sampleDimension(int64_t index, int dim) const {
    if (_sampleAtPixelCenter && (dim == 0 || dim == 1)) {
        return 0.5f;
    }
    if (dim == 0) {
        return RadicalInverse(dim, index >> _baseExponents[0]);
    } else if (dim == 1) {
        return RadicalInverse(dim, index / _baseScales[1]);
    } else {
        return ScrambledRadicalInverse(dim, index, permutationForDimension(dim));
    }
}

std::unique_ptr<Sampler> HaltonSampler::clone(int seed) {
    return std::unique_ptr<Sampler>(new HaltonSampler(*this));
}

PALADIN_END
