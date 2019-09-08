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
    // 生成质数进制随机重排表
    if (_radicalInversePermutations.empty()) {
        RNG rng;
        _radicalInversePermutations = ComputeRadicalInversePermutations(rng);
    }
    
    /**
     * kMaxResolution = 128(todo 了解一下kMaxResolution为何定位128)
     * 将halton序列的前两个维度映射到[0,1)^2中，并且超过kMaxResolution的部分用重复的halton序列
     * 没有超过kMaxResolution的维度不做处理
     * 又由halton的质数基底的性质可得
     * 我们需要找到i与j满足(2^i, 3^j)向量两个维度均大于std::min(res[idx], kMaxResolution) （res[idx]为第idx个维度的分辨率）
     * 由以下代码可以看出
     * 
     * _baseExponents[0]就是i
     * _baseExponents[1]就是j
     * _baseScales[0]就是2^i
     * _baseScales[1]就是3^j
     *
     * 为何要有如上的操作？
     */
    Vector2i res = sampleBounds.pMax - sampleBounds.pMin;
    // 如果分辨率有一个或两个维度大于kMaxResolution
    // 则在图像上使用重复的halton样本点
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
