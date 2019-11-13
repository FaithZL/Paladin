//
//  halton.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/8/29.
//

#ifndef halton_hpp
#define halton_hpp

#include "core/header.h"
#include "core/sampler.hpp"
#include "math/lowdiscrepancy.hpp"

PALADIN_BEGIN

/**
 * 霍尔顿采样器
 * 继承GlobalSampler，针对整个图像空间进行采样
 * 
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
 *
 * halton序列理论知识已经理解，根据当前像素样本索引获取样本全局索引的方式还没搞懂todo
 *
 *
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
    
    virtual neb::CJsonObject toJson() const override;
    
private:
    
    // 质数进制的随机重排表
    static std::vector<uint16_t> _radicalInversePermutations;
    
    // 在构造函数中有详细注释
    Point2i _baseScales, _baseExponents;
    
    // 暂时称为采样步长
    // 步长的定义为假设当前像素第一个样本的全局索引为idx，那么第二个样本的全局索引
    // 为idx + _sampleStride * 1，每次在该像素采样时，都是上一个样本的索引加上一个步长
    // 相关信息在getIndexForSample函数的注释中详细说明
    // _sampleStride = _baseScales[0] * _baseScales[1];
    int _sampleStride;
    
    // 储存数论倒数
    // 数论倒数的定义
    // 如果两个数a和b，它们的乘积关于模m余1，那么我们称它们互为关于模m的数论倒数
    // 2*3 mod 5 = 1，所以3是2关于5的数论倒数
    int _multInverse[2];
    
    // 用于储存当前采样的像素，这跟_currentPixel区别的一点
    // 主要在于，每次切换像素的时候需要_offsetForCurrentPixel
    // 之后就_sampleStride的步长进行采样，这个变量主要是为了记录之后计算了_offsetForCurrentPixel
    mutable Point2i _pixelForOffset = Point2i(std::numeric_limits<int>::max(),
                                             std::numeric_limits<int>::max());
    
    // 用于储存第一个落在该像素上的样本点的全局样本索引
    mutable int64_t _offsetForCurrentPixel;
    
    // 是否强制采样像素中心，如果为true，
    // 则生成的高维变量的前两个维度为0.5，采样像素中心
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
