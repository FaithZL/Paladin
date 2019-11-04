//
//  envmap.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/4.
//

#include "envmap.hpp"
#include "math/sampling.hpp"

PALADIN_BEGIN

EnvironmentMap::EnvironmentMap(const Transform &LightToWorld,
                               const Spectrum &power,
                               int nSamples, const std::string &texmap)
:Light((int)LightFlags::Infinite, LightToWorld, MediumInterface(),
nSamples) {
    
}

Spectrum EnvironmentMap::power() const {
    // todo 球面映射会产生扭曲所以这样估计通量是有误差的
    return Pi * _worldRadius * _worldRadius *
           Spectrum(_Lmap->lookup(Point2f(.5f, .5f), .5f),
                    SpectrumType::Illuminant);
}

Spectrum EnvironmentMap::Le(const RayDifferential &ray) const {
    Vector3f w = normalize(_worldToLight.exec(ray.dir));
    Point2f st(sphericalPhi(w) * Inv2Pi, sphericalTheta(w) * InvPi);
    return Spectrum(_Lmap->lookup(st), SpectrumType::Illuminant);
}

Spectrum EnvironmentMap::sampleLi(const Interaction &ref, const Point2f &u,
                                      Vector3f *wi, Float *pdf,
                                      VisibilityTester *vis) const {
    Float mapPdf;
    Point2f uv = _distribution->sampleContinuous(u, &mapPdf);
    if (mapPdf == 0) {
        return Spectrum(0.f);
    }
    Float theta = uv[1] * Pi, phi = uv[0] * 2 * Pi;
    Float cosTheta = std::cos(theta), sinTheta = std::sin(theta);
    Float sinPhi = std::sin(phi), cosPhi = std::cos(phi);
    Vector3f wiLight = Vector3f(sinTheta * cosPhi,
                                sinTheta * sinPhi,
                                cosTheta);
    *wi = _lightToWorld.exec(wiLight);

    *pdf = mapPdf / (2 * Pi * Pi * sinTheta);
    if (sinTheta == 0) {
        *pdf = 0;
    }

    *vis = VisibilityTester(ref, Interaction(ref.pos + *wi * (2 * _worldRadius),
                                             ref.time, mediumInterface));
    return Spectrum(_Lmap->lookup(uv), SpectrumType::Illuminant);
}

PALADIN_END
