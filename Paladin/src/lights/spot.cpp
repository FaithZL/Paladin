//
//  spot.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/18.
//

#include "spot.hpp"

PALADIN_BEGIN

SpotLight::SpotLight(shared_ptr<const Transform> LightToWorld,
                     const MediumInterface &mediumInterface, const Spectrum &I,
                     Float totalWidth, Float falloffStart)
: Light((int)LightFlags::DeltaPosition, LightToWorld, mediumInterface),
_pos(LightToWorld->exec(Point3f(0, 0, 0))),
_I(I),
_cosTotalWidth(std::cos(degree2radian(totalWidth))),
_cosFalloffStart(std::cos(degree2radian(falloffStart))) {
    
}

Spectrum SpotLight::sample_Li(const Interaction &ref, const Point2f &u,
                              Vector3f *wi, Float *pdf,
                              VisibilityTester *vis) const {
    *wi = normalize(_pos - ref.pos);
    *pdf = 1.f;
    *vis = VisibilityTester(ref, Interaction(_pos, ref.time, mediumInterface));
    return _I * falloff(-*wi) / distanceSquared(_pos, ref.pos);
}

Float SpotLight::falloff(const Vector3f &w) const {
    Vector3f wl = normalize(_worldToLight->exec(w));
    Float cosTheta = wl.z;
    if (cosTheta < _cosTotalWidth) {
        return 0;
    }
    if (cosTheta >= _cosFalloffStart) {
        return 1;
    }

    Float delta = (cosTheta - _cosTotalWidth) / (_cosFalloffStart - _cosTotalWidth);
    return (delta * delta) * (delta * delta);
}

// todo待推导
Spectrum SpotLight::power() const {
    return _I * _2Pi * (1 - .5f * (_cosFalloffStart + _cosTotalWidth));
}

Float SpotLight::pdf_Li(const Interaction &, const Vector3f &) const {
    return 0.f;
}

//"param" : {
//    "worldToLocal" : {
//        "type" : "translate",
//        "param" : [1,0,1]
//    },
//    "I" : {
//        "colorType" : 1,
//        "color" : [0.1, 0.9, 0.5]
//    },
//    "falloffStart" : 45,
//    "totalAngle" : 60
//}
CObject_ptr createSpot(const nloJson &param, const Arguments &lst) {
    nloJson l2w_data = param.value("lightToWorld", nloJson());
    auto l2w = shared_ptr<const Transform>(createTransform(l2w_data));
    nloJson Idata = param.value("I", nloJson::object());
    Spectrum I = Spectrum::FromJson(Idata);
    Float falloffStart = param.value("falloffStart", 45.f);
    Float totalAngle = param.value("totalAngle", 60.f);
    auto ret = new SpotLight(l2w, nullptr, I, totalAngle, falloffStart);
    return ret;
}

REGISTER("spot", createSpot)

PALADIN_END
