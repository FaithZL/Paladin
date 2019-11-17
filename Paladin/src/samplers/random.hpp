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
    
    virtual void startPixel(const Point2i &) override;
    
    virtual Float get1D() override;
    
    virtual Point2f get2D() override;
    
    virtual nebJson toJson() const override;
    
    virtual std::unique_ptr<Sampler> clone(int seed) override;
    
private:
    RNG _rng;
};

USING_STD

CObject_ptr createRandomSampler(const nebJson &param);

PALADIN_END

#endif /* random_hpp */
