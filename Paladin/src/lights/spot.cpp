//
//  spot.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/18.
//

#include "spot.hpp"
#include "math/sampling.hpp"
#include "core/bxdf.hpp"

PALADIN_BEGIN

SpotLight::SpotLight(const Transform  * LightToWorld,
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

Spectrum SpotLight::sample_Li(DirectSamplingRecord *rcd,
                              const Point2f &u,
                              const Scene &scene) const {
    Vector3f wi = _pos - rcd->ref();
    rcd->updateTarget(wi, 1.0f);
    auto vis = rcd->getVisibilityTester();
    auto ret = vis.unoccluded(scene) ?
            _I * falloff(-rcd->dir()) / wi.lengthSquared() : 0;
    return ret;
}

Spectrum SpotLight::sample_Le(const Point2f &u1, const Point2f &u2,
                                     Float time, Ray *ray, Normal3f *nLight,
                                     Float *pdfPos, Float *pdfDir) const {
    *pdfPos = 1;
    *pdfDir = uniformConePdf(_cosTotalWidth);
    *ray = Ray(_pos, uniformSampleCone(u2, _cosTotalWidth), Infinity, time, mediumInterface.inside);
    *nLight = Normal3f(ray->dir);
    return _I * falloff(ray->dir);
}

void SpotLight::pdf_Le(const Ray &ray, const Normal3f &nLight,
                       Float *pdfPos, Float *pdfDir) const {
    *pdfPos = 0;
    *pdfDir = (Frame::cosTheta(_worldToLight->exec(ray.dir)) >= _cosTotalWidth) ?
                uniformConePdf(_cosTotalWidth) : 0;
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

Float SpotLight::pdf_Li(const DirectSamplingRecord &) const {
    return 0.f;
}

//"param" : {
//    "transform" : {
//        "type" : "translate",
//        "param" : [1,0,1]
//    },
//    "I" : {
//        "colorType" : 1,
//        "color" : [0.1, 0.9, 0.5]
//    },
//    "falloffStart" : 45,
//    "totalAngle" : 60,
//    "scale" : 1.f,
//}
CObject_ptr createSpot(const nloJson &param, const Arguments &lst) {
    nloJson l2w_data = param.value("transform", nloJson());
    auto l2w = createTransform(l2w_data);
    nloJson Idata = param.value("I", nloJson::object());
    nloJson scale = param.value("scale", 1.f);
    Spectrum I = Spectrum::FromJson(Idata);
    I *= (Float)scale;
    Float totalAngle = param.value("totalAngle", 60.f);
    Float falloffStart = param.value("falloffStart", 0);
    falloffStart = falloffStart == 0 ? totalAngle * 0.8 : falloffStart;
    auto ret = new SpotLight(l2w, nullptr, I, totalAngle, falloffStart);
    return ret;
}

REGISTER("spot", createSpot)

PALADIN_END
