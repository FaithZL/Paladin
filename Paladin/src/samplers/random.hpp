//
//  random.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef random_hpp
#define random_hpp

#include "core/header.h"
#include "core/sampler.hpp"

PALADIN_BEGIN

class RandomSampler : public Sampler {
public:
    RandomSampler(int ns, int seed = 0)
    : Sampler(ns),
    _rng(seed) {
        
    }
    
    virtual void startPixel(const Point2i &);
    
    virtual Float get1D();
    
    virtual Point2f get2D();
    
    virtual neb::CJsonObject toJson() const override;
    
    virtual std::unique_ptr<Sampler> clone(int seed);
    
private:
    RNG _rng;
};

USING_STD

Serialize_ptr createRandomSampler(const nebJson &param);

PALADIN_END

#endif /* random_hpp */
