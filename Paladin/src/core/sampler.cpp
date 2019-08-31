//
//  sampler.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "sampler.hpp"


PALADIN_BEGIN

Sampler::Sampler(int64_t samplesPerPixel)
: samplesPerPixel(samplesPerPixel) {
    
}

void Sampler::startPixel(const Point2i &p) {
    _currentPixel = p;
    _currentPixelSampleIndex = 0;
    _array1DOffset = _array2DOffset = 0;
}



PALADIN_END
