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
     * kMaxResolution = 128(todo 了解一下kMaxResolution为何定为128)
     * 定义 a = std::min(res[idx], kMaxResolution) （res[idx]为第idx个维度的分辨率）
     * 将halton序列的前两个维度映射从[0,1)^2中，并且超过a的部分用重复的halton序列
     * 没有超过 a 的维度不做处理
     * 又由halton的质数基底的性质可得
     * 我们需要找到i与j满足(2^i, 3^j)向量两个维度均大于 a
     * 由以下代码可以看出
     * 
     * _baseExponents[0]就是i
     * _baseExponents[1]就是j
     * _baseScales[0]就是2^i  128  (a = 128)
     * _baseScales[1]就是3^j  243  (a = 128)
     *
     * 将 [0,1)^2 空间中的样本映射到 [0,128) * [0,243) 空间中
     * 以第一维度x方向为例x[i]代表，x方向上第i个样本，inverse函数表示逐位反转，10变为0.01
     * x[0] = 0, 
     * x[1] = inverse(1)(base2) = 0.1(base2) = 1/2(base10)
     * x[2] = inverse(10)(base2) = 0.01(base2) = 1/4(base10)
     *
     * 为了将x从 [0,1) 映射到 [0,128) 区间
     * 各个x[i]的值需要乘以一个128(2^7)，对应_baseExponents[0]为7
     *
     * 为何要有如上的操作？
     * 这个分辨率的限制，可以使浮点精度控制在一个可以接受的范围内（数字越大，浮点精度会越低）
     * 
     */
    Vector2i res = sampleBounds.pMax - sampleBounds.pMin;
    
    // 如果分辨率有一个或两个维度大于a
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

/**
 * 通过当前像素的样本局部索引，计算出样本的全局索引
 * 首先来说一个前置知识点
 *    a  Base为2  f2(a)      f2为基反转
 *    0   0       0
 *    1   1       0.1= 1/2
 *    2   10      0.01= 1/4
 *    3   11      0.11= 3/4
 *    4   100     0.001= 1/8
 *    5   101     0.101= 5/8
 * 
 * 上图表所示，如果把一个坐标轴x分看做一个区间range，则按照顺序采样，
 * 第一个落在range内的样本是落在range的左端点。
 * 如果把一个坐标轴分为两个区间r1,r2，第一个落在区间r1跟r2上的点均为，r1 r2的左端点
 * 同样，把区间分为4分，第一个落在四个区间上的点，分别都是在区间的左端点(0, 1/2, 1/4, 3/4)
 *  ---->>>>> 归纳一下，把区间分为2^i个区间，落在每个区间的第一个点都在区间的左端点
 * 
 * 继续观察以上序列，如果把区间分成4份，第一个子区间为r[0]，第二个子区间为r[1]
 * 第一个落在r[0]的点的样本索引为0
 * 第二个落在r[0]的点的样本索引为4
 * 
 * 第一个落在r[1]的点的样本索引为1
 * 第二个落在r[1]的点的样本索引为5
 * 可以看出步长stride为4，我们把同一个区间内，索引最接近的两个样本的索引差称为步长stride
 * 多观察几组数据，可以归纳出：将一个区间平均分为2^i份，则stride为2^i
 * (emmm, 其实数学上更加严谨的做法是，先猜想，后证明，证明过程就不写了，毕竟不是主线任务，我也没搞过)
 * 
 * 综上所述，以上性质可以延伸到y轴进行观察(base 为 3) strideY = 3^j
 * 总的采样步长stride应该为xy两个方向上的采样步长的乘积(想象一下，不难理解)
 * 所以 有了 _sampleStride = _baseScales[0] * _baseScales[1];这行代码
 * 
 * 如上所述，我们将将 [0,1)^2 空间中的样本映射到 [0,2^i) * [0,3^j) 空间中
 * 相当于每个样本的x乘以2^i,y乘以3^j,可以将以上我们归纳的信息应用起来了
 * 同样以x轴为例，原始值为 x1 = 0.d1(x1)d2(x1)...dn(x1) 二进制
 * 乘以2^i之后，相当于小数点向右移动i位，去掉小数点后的尾数取整，就得到像素的x坐标
 * 以上描述可以简化为表达式
 * inverse_in_2(x) = idx mod 2^i,
 * inverse_in_3(y) = idx mod 3^j,
 * inverse_in_2(x)为2进制下x的反转(例如，1011-> 1101),y不再赘述
 * 其中idx为第一个落在当前像素的样本的全局索引
 * todo 这个函数的计算过程还是没有完全搞懂，甚至pbrbook中都略过了，
 * multiplicativeInverse函数是真jb看不懂，先搞完主线在回头搞搞
 *
 * @param  sampleNum 样本的局部索引
 * @return           样本的全局索引
 */
int64_t HaltonSampler::getIndexForSample(int64_t sampleNum) const {
    // 如果采样器切换了需要采样的像素，重新计算_offsetForCurrentPixel
    if (_currentPixel != _pixelForOffset) {
        _offsetForCurrentPixel = 0;
        if (_sampleStride > 1) {
            // 超过kMaxResolution的部分进行重新映射
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
