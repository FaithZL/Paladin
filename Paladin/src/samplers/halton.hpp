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

/*
 渲染时会把图片分割为若干个块(tile)，每个tile使用一个采样器
 所以构造函数会接收一个AABB参数

 
 */
class HaltonSampler : public GlobalSampler {
    
public:
    
    HaltonSampler(int nsamp, const AABB2i &sampleBounds,
                  bool sampleAtCenter = false);
    
    virtual int64_t getIndexForSample(int64_t sampleNum) const;
    
    virtual Float sampleDimension(int64_t index, int dimension) const;
    
    virtual std::unique_ptr<Sampler> clone(int seed);
    
private:
    
    static std::vector<uint16_t> _radicalInversePermutations;
    
    Point2i _baseScales, _baseExponents;
    
    int _sampleStride;
    
    int _multInverse[2];
    
    mutable Point2i _pixelForOffset = Point2i(std::numeric_limits<int>::max(),
                                             std::numeric_limits<int>::max());
    
    mutable int64_t _offsetForCurrentPixel;
    // Added after book publication: force all image samples to be at the
    // center of the pixel area.
    bool _sampleAtPixelCenter;
    
    // HaltonSampler Private Methods
    const uint16_t *permutationForDimension(int dim) const {
        if (dim >= PrimeTableSize)
            COUT << StringPrintf("HaltonSampler can only sample %d "
                                       "dimensions.", PrimeTableSize);
        return &_radicalInversePermutations[PrimeSums[dim]];
    }
};

PALADIN_END

#endif /* halton_hpp */
