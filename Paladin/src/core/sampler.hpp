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

    virtual ~Sampler();

    Sampler(int64_t samplesPerPixel);
    
    virtual void startPixel(const Point2i &p);
    
    virtual Float get1D() = 0;
    
    virtual Point2f get2D() = 0;
    
    CameraSample getCameraSample(const Point2i &pRaster);
    
    void request1DArray(int n);
    
    void request2DArray(int n);
    
    virtual int roundCount(int n) const {
        return n;
    }
    
    const Float *get1DArray(int n);

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
    
    Point2i _currentPixel;
    int64_t _currentPixelSampleIndex;
    std::vector<int> _samples1DArraySizes, _samples2DArraySizes;
    std::vector<std::vector<Float>> _sampleArray1D;
    std::vector<std::vector<Point2f>> _sampleArray2D;
    
private:

    size_t _array1DOffset, _array2DOffset;
};

PALADIN_END

#endif /* sampler_hpp */
