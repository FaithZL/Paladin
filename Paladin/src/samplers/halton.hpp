//
//  halton.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/8/29.
//

#ifndef halton_hpp
#define halton_hpp

#include "header.h"
#include "sampler.hpp"
#include "lowdiscrepancy.hpp"

PALADIN_BEGIN

/**
 * 霍尔顿采样器
 * 在介绍霍尔顿采样器之前我们先介绍一下低差异序列
 * 
 * 低差异序列可以生成更加均匀的样本，从而提高采样效率
 * 
 * 霍尔顿序列是低差异序列的一种
 * 基于radical inverse(暂时不知道如何翻译这个名词)
 * 
 * 任何一个正整数a都可以表示为一个b进制数dm(a)...d2(a)d1(a)
 * a = ∑[i=1, m]di(a)*b^(i-1)
 * 所有位数di(a)∈[0, b-1]
 * radical inverse函数fb(a) = 0.d1(a)d2(a)...dm(a)
 * 因此di(a)对于数字的贡献为di(a)/(b^(i-1))
 * 
 * 最简单的低差异序列为van der Corput 序列，这是一个一维序列
 * 取b值为1
 * 以下图表展示了
 *    a  Base为2  f2(a)
 *    0   0       0
 *    1   1       0.1= 1/2
 *    2   10      0.01= 1/4
 *    3   11      0.11= 3/4
 *    4   100     0.001= 1/8
 *    5   101     0.101= 5/8
 * 细心的话，可以看得出来这是递归的从中点开始分割一条一维线段
 * 这个序列的差异为D[*N](P) = O(log(2,N)/N)，N如果无穷大，差异为零，符合我们的预期
 * 此概念扩展到n维可得D[*N](P) = O((log(2,N)^n)/N)
 * 
 * 为了生成n维的霍尔顿序列，每个维度提供不同的基底(base)。
 * base为依次递增的质数(2,3,5,7,11,13.....)
 * 
 * 可以表示为 x(a) = (f2(a), f3(a), f5(a), f7(a), f11(a).....)
 * 这样生成出来的点，差异很低
 * 至于为何是质数，这......这尼玛还是要问数学家是怎么研究出来的，这已经超出我的能力范围了，
 * 
 * 有了以上思路，我可以开始搞起了(写代码实现霍尔顿采样器)
 * 
 * 霍尔顿序列的有个缺点：
 * 在样本矢量的较高维度中，样本值的投影开始呈现规则结构，太过于规则了，对于渲染效果来说不好
 * 
 * 这可以通过添加一个轻微的扰动，解决这个问题
 */
class HaltonSampler : public GlobalSampler {
    
public:
    
    /**
     * 渲染时会把图片分割为若干个块(tile)，每个tile使用一个采样器
     * 所以构造函数会接收一个AABB参数
     */
    HaltonSampler(int spp, const AABB2i &sampleBounds, bool sampleAtCenter = false);
    
    virtual int64_t getIndexForSample(int64_t sampleNum) const;
    
    virtual Float sampleDimension(int64_t index, int dimension) const;
    
    virtual std::unique_ptr<Sampler> clone(int seed);
    
private:
    
    // 质数进制的随机重排表
    static std::vector<uint16_t> _radicalInversePermutations;
    
    // 在构造函数中有详细注释
    Point2i _baseScales, _baseExponents;
    
    int _sampleStride;
    
    int _multInverse[2];
    
    mutable Point2i _pixelForOffset = Point2i(std::numeric_limits<int>::max(),
                                             std::numeric_limits<int>::max());
    
    mutable int64_t _offsetForCurrentPixel;
    // Added after book publication: force all image samples to be at the
    // center of the pixel area.
    bool _sampleAtPixelCenter;
    
    /**
     * 返回对应维度重排之后的数组
     * @param  dim 维度
     * @return     [description]
     */
    const uint16_t *permutationForDimension(int dim) const {
        if (dim >= PrimeTableSize)
            COUT << StringPrintf("HaltonSampler can only sample %d "
                                       "dimensions.", PrimeTableSize);
        return &_radicalInversePermutations[PrimeSums[dim]];
    }
};

PALADIN_END

#endif /* halton_hpp */
