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

PALADIN_BEGIN

/*
 Sampler类的作用主要是生成一系列的n维向量[0,1)^n
 当然，这个维度可以变化
 例如样本可以为[x,y,t,u,v,[u0,u1],[u2,u3],[u4,u5],[u6,u7]]
 x,y表示采样像素的偏移值，t表示ray的时间，uv表示采样透镜的位置
 之后的四组二维随机向量，光照传输算法会申请一系列的样本，表示用于采样area光源上的四个点
 
 
 */
class Sampler {

public:

    virtual ~Sampler() {
        
    }

    Sampler(int64_t samplesPerPixel);
    
    // 对某个像素点开始采样
    virtual void startPixel(const Point2i &p);
    
    virtual Float get1D() = 0;
    
    virtual Point2f get2D() = 0;
    
    CameraSample getCameraSample(const Point2i &pRaster);
    
    // 申请一个长度为n的一维随机变量数组
    void request1DArray(int n);
    
    // 申请一个长度为n的二维随机变量数组
    void request2DArray(int n);
    
    virtual int roundCount(int n) const {
        return n;
    }
    
    // 获取包含n个样本的一维数组，需要根据之前request的值做校验
    const Float *get1DArray(int n);
    
    // 获取包含n个样本的二维数组，需要根据之前request的值做校验
    const Point2f *get2DArray(int n);

    virtual bool startNextSample();

    virtual std::unique_ptr<Sampler> clone(int seed) = 0;

    virtual bool setSampleNumber(int64_t sampleNum);

    std::string stateString() const {
        return StringPrintf("(%d,%d), sample %", _currentPixel.x,
                            _currentPixel.y, _currentPixelSampleIndex);
    }

    int64_t currentSampleNumber() const { 
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

PALADIN_END

#endif /* sampler_hpp */
