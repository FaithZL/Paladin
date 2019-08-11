//
//  medium.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/7/16.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef medium_hpp
#define medium_hpp

#include "header.h"

PALADIN_BEGIN

class PhaseFunction {
public:
    // PhaseFunction Interface
    virtual ~PhaseFunction();
    virtual Float p(const Vector3f &wo, const Vector3f &wi) const = 0;
    virtual Float sample_p(const Vector3f &wo, Vector3f *wi,
                           const Point2f &u) const = 0;
    virtual std::string toString() const = 0;
};

// 介质
class Medium {
public:
    // Medium Interface
    virtual ~Medium() {}
    virtual Spectrum tr(const Ray &ray, Sampler &sampler) const = 0;
    virtual Spectrum sample(const Ray &ray, Sampler &sampler,
                            MemoryArena &arena,
                            MediumInteraction *mi) const = 0;
};


// 两个介质的相交处，nullptr表示真空
struct MediumInterface {
    MediumInterface() : inside(nullptr), outside(nullptr) {}

    MediumInterface(const Medium *medium) : inside(medium), outside(medium) {

    }

    MediumInterface(const Medium *inside, const Medium *outside)
    :inside(inside),
    outside(outside) {

    }
    
    bool isMediumTransition() const {
        return inside != outside;
    }
    // 内部的介质
    const Medium *inside;
    
    // 外部的介质
    const Medium *outside;
};

PALADIN_END

#endif /* medium_hpp */
