//
//  light.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef light_hpp
#define light_hpp

#include "core/header.h"
#include "core/interaction.hpp"
#include "math/transform.hpp"

PALADIN_BEGIN


enum class LightFlags {
    DeltaPosition = 1,
    DeltaDirection = 2,
    Area = 4,
    Infinite = 8
};

inline bool isDeltaLight(int flags) {
    return flags & (int)LightFlags::DeltaPosition ||
           flags & (int)LightFlags::DeltaDirection;
}

class Light {
    
public:
    
    virtual ~Light() {

    }

    Light(int flags, const Transform &LightToWorld,
          const MediumInterface &mediumInterface, int nSamples = 1)
        
    : flags(flags),
    nSamples(std::max(1, nSamples)),
    mediumInterface(mediumInterface),
    _lightToWorld(LightToWorld),
    _worldToLight(LightToWorld.getInverse()) {

    }
    
    virtual Spectrum sampleLi(const Interaction &ref, const Point2f &u,
                               Vector3f *wi, Float *pdf,
                               VisibilityTester *vis) const = 0;

    virtual Spectrum power() const = 0;

    virtual void preprocess(const Scene &scene) {

    }

    virtual Spectrum le(const RayDifferential &r) const;

    virtual Float pdfLi(const Interaction &ref, const Vector3f &wi) const = 0;

    virtual Spectrum sampleLe(const Point2f &u1, const Point2f &u2, Float time,
                               Ray *ray, Normal3f *nLight, Float *pdfPos,
                               Float *pdfDir) const = 0;

    virtual void pdfLe(const Ray &ray, const Normal3f &nLight, Float *pdfPos,
                        Float *pdfDir) const = 0;
    
    const int flags;
    const int nSamples;
    const MediumInterface mediumInterface;
    
protected:

    const Transform _lightToWorld, _worldToLight;
};

class AreaLight : public Light {
public:
    AreaLight(const Transform &LightToWorld, 
            const MediumInterface &mi,
            int nSamples)
    :Light((int)LightFlags::Area, LightToWorld, mi, nSamples) {

    }

    virtual Spectrum L(const Interaction &intr, const Vector3f &w) const = 0;
};

class VisibilityTester {
public:
    VisibilityTester() {}
    VisibilityTester(const Interaction &p0, const Interaction &p1)
    : _p0(p0), _p1(p1) {}
    const Interaction &P0() const { return _p0; }
    const Interaction &P1() const { return _p1; }
    bool Unoccluded(const Scene &scene) const;
    Spectrum Tr(const Scene &scene, Sampler &sampler) const;
    
private:
    Interaction _p0, _p1;
};

PALADIN_END

#endif /* light_hpp */
