//
//  stratified.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/9/3.
//

#ifndef stratified_hpp
#define stratified_hpp

#include "header.h"
#include "sampler.hpp"

PALADIN_BEGIN

class StratifiedSampler : public PixelSampler {
    
public:
    StratifiedSampler(int xPixelSamples, int yPixelSamples, bool jitterSamples,
                      int nSampledDimensions)
    : PixelSampler(xPixelSamples * yPixelSamples, nSampledDimensions),
    _xPixelSamples(xPixelSamples),
    _yPixelSamples(yPixelSamples),
    _jitterSamples(jitterSamples) {
        
    }
    void StartPixel(const Point2i &);
    
    std::unique_ptr<Sampler> clone(int seed);
    
private:
    
    const int _xPixelSamples, _yPixelSamples;
    
    const bool _jitterSamples;
};

PALADIN_END

#endif /* stratified_hpp */
