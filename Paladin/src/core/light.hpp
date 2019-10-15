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
#include "core/spectrum.hpp"

PALADIN_BEGIN

黑体是一种完美的发射器:它能尽可能有效地将电能转化为电磁辐射。
虽然真正的黑体在物理上是不存在的，但一些发射器表现出接近黑体的行为。
黑体的发射也有一个有用的封闭表达式(解析解)
它是参数为温度和波长的函数，这对非黑体发射体的建模很有用。

黑体之所以这样命名，是因为它吸收所有的入射光，没有任何反射
看起来就是全黑，从直观上看，完美吸收体也是完美发射体的原因在于，吸收是发射的反向操作。
因此，如果时间倒转，所有被完全吸收的能量都会被完全有效地重新释放

普朗克定律给出了黑体发射的辐射率，它是波长和温度的函数，单位是开尔文:

                    2 h c^2
Le(λ,T) = -----------------------------
            λ^5 [e^(h c / λ kb T) - 1]

其中 h 为普兰克常数 h = 6.62606957 * 10^34 Js
c 为光在真空中传播的速度 c = 299792458 m/s
kb 为玻尔兹曼常数 kb = 1.3806488 * 10^-23 J/K




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

    virtual Spectrum le(const RayDifferential &r) const {
        return Spectrum(0.f);
    }

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
    VisibilityTester() {
        
    }
    
    VisibilityTester(const Interaction &p0, const Interaction &p1)
    : _p0(p0), _p1(p1) {
        
    }
    
    const Interaction &P0() const {
        return _p0;
    }
    
    const Interaction &P1() const {
        return _p1;
    }
    
    bool Unoccluded(const Scene &scene) const;
    
    Spectrum Tr(const Scene &scene, Sampler &sampler) const;
    
private:
    Interaction _p0, _p1;
};

PALADIN_END

#endif /* light_hpp */
