//
//  sampler.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef sampler_hpp
#define sampler_hpp

#include "header.h"
#include "rng.h"
PALADIN_BEGIN

/**
 * Sampler类的作用主要是生成一系列的n维向量[0,1)^n
 * 当然，这个维度可以变化
 * 例如样本可以为[x,y,t,u,v,[u0,u1],[u2,u3],[u4,u5],[u6,u7]]
 * x,y表示采样像素的偏移值，t表示ray的时间，uv表示采样透镜的位置
 * 之后的四组二维随机向量，光照传输算法会申请一系列的样本，表示用于采样area光源上的四个点
 * 
 * 首先理解一下"差异"的概念
 * 在n维空间的[0,1)^n区间中
 * 体积为v的形状中包含的样本数量/样本总数量，越接近v，则差异越低
 * 更加详细的说法：在某个一维区间生成随机点，区间内指定了子区间(子区间可以从总区间内任何地方开始或结束)
 * 子区间的长度与在子区间内的点的个数越接近线性关系，则差异越低，也就是越均匀
 * 此概念可以扩展到二维，三维甚至高维
 * 
 * 差异越低的序列，则表示该序列越均匀，均匀的采样好处在于可以更好的收敛到可接受的值
 * 但太过于均匀的序列进行采样会产生规则的走样，不均匀的序列容易产生噪点
 * 从感官上来说，图片走样比噪点更让人讨厌
 * 
 * 所以采样方式会有很多种，以便于应对各种不同的场景
 * 目前在初学阶段，暂时只实现随机采样，分层采样，霍尔顿采样
 *
 */
class Sampler {

public:

    virtual ~Sampler() {
        
    }

    Sampler(int64_t samplesPerPixel);
    
    // 对某个像素点开始采样
    virtual void startPixel(const Point2i &p);
    
    /**
     * 返回一维随机变量，每次调用之后，维度下标都会自增
     * 下次调用时，获取下个维度的随机变量
     * @return   一维随机变量
     */
    virtual Float get1D() = 0;
    
    /**
     * 返回二维随机变量，每次调用之后，维度下标都会自增
     * 下次调用时，获取下个维度的随机变量
     * @return   二维随机变量
     */
    virtual Point2f get2D() = 0;
    
    CameraSample getCameraSample(const Point2i &pRaster);
    
    // 申请一个长度为n的一维随机变量数组
    void request1DArray(int n);
    
    // 申请一个长度为n的二维随机变量数组
    void request2DArray(int n);
    
    virtual int roundCount(int n) const {
        return n;
    }
    
    /**
     * 获取包含n个样本的一维数组，需要根据之前request的值做校验
     * 返回一个数组，数组元素都为同一个分布，同一个维度的不同样本
     * @param  n 
     * @return   数组首地址
     */
    const Float *get1DArray(int n);
    
    /**
     * 获取包含n个样本的一维数组，需要根据之前request的值做校验
     * 返回一个数组，数组元素都为同一个分布，同一个维度的不同样本
     * 这点需要跟get2D函数区分开
     * 例如，如果要对一个光源表面进行采样
     * 则最好调用该函数获取一系列同一个维度的样本
     * @param  n 
     * @return   数组首地址
     */
    const Point2f *get2DArray(int n);
    
    // 开始下一个样本，返回值为该像素是否采样完毕
    virtual bool startNextSample();

    virtual std::unique_ptr<Sampler> clone(int seed) = 0;

    virtual bool setSampleIndex(int64_t sampleNum);

    std::string stateString() const {
        return StringPrintf("(%d,%d), sample %", _currentPixel.x,
                            _currentPixel.y, _currentPixelSampleIndex);
    }

    int64_t currentSampleIndex() const { 
        return _currentPixelSampleIndex; 
    }
    
    const int64_t samplesPerPixel;
    
protected:

    // 当前处理的像素点
    Point2i _currentPixel;
    
    // 当前处理的像素样本索引
    int64_t _currentPixelSampleIndex;
    
    // 用于储存一维样本数量的列表
    std::vector<int> _samples1DArraySizes;
    
    // 用于储存二维样本数量的列表
    std::vector<int> _samples2DArraySizes;
    
    // 用于储存一维样本的列表，列表的元素也是一个列表，暂时称为二级列表
    // 二级列表在内存布局上是一维数组，但在使用中可以理解为二维数组
    std::vector<std::vector<Float>> _sampleArray1D;
    
    // 用法类似一维样本，不再赘述
    std::vector<std::vector<Point2f>> _sampleArray2D;
    
private:
    
    size_t _array1DOffset;
    size_t _array2DOffset;
};

/**
 * 像素采样器
 * 为一个像素生成所有维度的样本
 * 
 * 在渲染一个像素之前，我们是无法预知一个像素的随机样本的维度的
 * 所以在构造像素采样器时，先指定一个维度参数nSampledDimensions，
 * 生成对应的维度数据数组，如果数据完全消耗之后，再需要使用随机数
 * 则通过rng生成
 * 
 * sample1D[dim][pixelSample]这类索引方式看起来有点奇怪，比较违反直觉
 * 似乎sample1D[pixelSample][dim]的索引方式更加科学
 * 
 * 不这么做是有原因的是：
 * 同一维数在不同样本上的值在内存上是连续的，这在生成样本的时候会更加方便。
 */
class PixelSampler : public Sampler {
    
public:
    
    PixelSampler(int64_t samplerPerPixel, int nSampledDimensions);
    
    virtual bool startNextSample();
    
    virtual bool setSampleIndex(int64_t num);
    
    virtual Float get1D();
    
    virtual Point2f get2D();
    
protected:
    
    std::vector<std::vector<Float>> _samples1D;
    std::vector<std::vector<Point2f>> _samples2D;
    
    int _curDimension1D;
    int _curDimension2D;
    
    RNG _rng;
    
};


/**
 * 全局采样器不是基于像素的，是针对整个图像空间进行采样
 * 对每个像素采样时，要通过当前像素以及当前像素的样本索引计算出全局的样本索引
 * 然后通过全局的样本索引以及样本维度获取样本值，所以多了两个函数
 * getIndexForSample
 * sampleDimension
 * 这两个函数需要重点理解一下
 *
 * 如下例子，霍尔顿序列
 *
 * 放置一些样本在2x3的像素中
 * global index       [0, 1)^2 sample coordinates     Pixel sample coordinates
 * 0                 (0.000000, 0.000000)              (0.000000, 0.000000)
 * 1                 (0.500000, 0.333333)              (1.000000, 1.000000)
 * 2                 (0.250000, 0.666667)              (0.500000, 2.000000)
 * 3                 (0.750000, 0.111111)              (1.500000, 0.333333)
 * 4                 (0.125000, 0.444444)              (0.250000, 1.333333)
 * 5                 (0.625000, 0.777778)              (1.250000, 2.333333)
 * 6                 (0.375000, 0.222222)              (0.750000, 0.666667)
 * 7                 (0.875000, 0.555556)              (1.750000, 1.666667)
 * 8                 (0.062500, 0.888889)              (0.125000, 2.666667)
 * 9                 (0.562500, 0.037037)              (1.125000, 0.111111)
 * 10                (0.312500, 0.370370)              (0.625000, 1.111111)
 * 11                (0.812500, 0.703704)              (1.625000, 2.111111)
 * 12                (0.187500, 0.148148)              (0.375000, 0.444444)
 *
 * 
 */
class GlobalSampler : public Sampler {
    
public:
    GlobalSampler(int64_t samplesPerPixel) : Sampler(samplesPerPixel) {
        
    }
    
    virtual bool startNextSample();
    
    virtual void startPixel(const Point2i &p);
    
    virtual bool setSampleIndex(int64_t sampleNum);
    
    virtual Float get1D();
    
    virtual Point2f get2D();
    
    /**
     * 以上图表为例，如果当前像素为(0,2)，
     * 则该函数返回该第一个出现在该像素上的样本索引
     * getIndexForSample(0)应该返回2，返回第一个出现在该像素样本上的索引
     * getIndexForSample(1)应该返回8，返回第二个出现在该像素上的样本索引
     * 
     * @param  sampleNum 出现在当前像素的第sampleNum个样本
     * @return           样本在全局中的索引
     */
    virtual int64_t getIndexForSample(int64_t sampleNum) const = 0;
    
    /**
     * 返回指定索引跟维度的样本值
     * 是指像素内的偏移值，而不是初始的[0,1)^2空间的样本值
     * 以上图表为例 sampleDimension(4,1)，返回0.3333333
     * 先根据索引4找到(0.250000, 1.333333)，根据维度找出1.333333，然后算出偏移值0.3333333
     * 
     * @param  index     样本索引
     * @param  dimension 对应维度
     * @return           样本值
     */
    virtual Float sampleDimension(int64_t index, int dimension) const = 0;
    
private:

    // 当前样本的维度
    int _dimension;

    // 当前样本的全局索引
    int64_t _globalIndex;

    // 样本数组开始的维度，前五个样本生成的是相机样本
    static const int _arrayStartDim = 5;

    // 样本数组结束的维度
    int _arrayEndDim;
};



PALADIN_END




#endif /* sampler_hpp */
