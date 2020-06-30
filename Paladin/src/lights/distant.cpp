//
//  distant.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/18.
//

#include "distant.hpp"
#include "math/sampling.hpp"

PALADIN_BEGIN

DistantLight::DistantLight(const Transform * LightToWorld, const Spectrum &L,
                           const Vector3f &wLight)
: Light((int)LightFlags::DeltaDirection, LightToWorld, MediumInterface()),
_L(L),
_wLight(normalize(LightToWorld->exec(wLight))) {
    
}

Spectrum DistantLight::sample_Le(const Point2f &u1, const Point2f &u2,
                                     Float time, Ray *ray, Normal3f *nLight,
                                     Float *pdfPos, Float *pdfDir) const {
    Vector3f v1,v2;
    coordinateSystem(_wLight, &v1, &v2);
    Point2f tmp = uniformSampleDisk(u1);
    Point3f pDisk = _worldCenter + _worldRadius * (tmp.x * v1 + tmp.y * v2);
    Point3f ori = pDisk + _wLight * _worldRadius;
    *ray = Ray(ori, -_wLight, Infinity, time);
    
    *nLight = Normal3f(-_wLight);
    *pdfPos = 1 / (Pi * _worldRadius * _worldRadius);
    *pdfDir = 1;
    return _L;
}

void DistantLight::pdf_Le(const Ray &ray, const Normal3f &nLight,
                              Float *pdfPos, Float *pdfDir) const {
    *pdfPos = 1 / (Pi * _worldRadius * _worldRadius);
    *pdfDir = 0;
}

Spectrum DistantLight::sample_Li(const Interaction &ref, const Point2f &u,
                                 Vector3f *wi, Float *pdf,
                                 VisibilityTester *vis) const {
    *wi = _wLight;
    // 与点光源类似
    // 在ref处采样方向光源
    // 为了统一MC积分的格式：f(x)/p(x)，所以把p(x)赋值为1，
    // 这个问题想象起来可能比较抽象
    *pdf = 1;
    // 沿着光线方向，找到一个场景以外的点，用于检测遮挡
    Point3f pOutside = ref.pos + _wLight * (2 * _worldRadius);
    *vis = VisibilityTester(ref, Interaction(pOutside, ref.time, mediumInterface));
    return _L;
}

Spectrum DistantLight::sample_Li(DirectSamplingRecord *rcd,
                                 const Point2f &u,
                                 const Scene &scene) const {
    Vector3f dir = _wLight * (2 * _worldRadius);
    rcd->updateTarget(dir, 1.f);
    auto vis = rcd->getVisibilityTester();
    auto ret = vis.unoccluded(scene) ? _L : 0;
    return ret;
}

Spectrum DistantLight::power() const {
    return _L * Pi * _worldRadius * _worldRadius;
}

Float DistantLight::pdf_Li(const Interaction &, const Vector3f &) const {
    return 0.f;
}

Float DistantLight::pdf_Li(const DirectSamplingRecord &) const {
    return 0.f;
}

// "data" : {
//    "type" : "distant",
//    "param" : {
//        "transform" : {
//            "type" : "translate",
//            "param" : [1,0,1]
//        },
//        "L" : {
//            "colorType" : 1,
//            "color" : [0.1, 0.9, 0.5]
//        },
//        "wLight" : [1,1,1],
//        "scale" : 1.f
//    }
//}
CObject_ptr createDistantLight(const nloJson &param, const Arguments &lst) {
    nloJson l2w_data = param.value("transform", nloJson());
    auto l2w = createTransform(l2w_data);
    nloJson Ldata = param.value("L", nloJson::object());
    nloJson scale = param.value("scale", 1.f);
    Spectrum L = Spectrum::FromJson(Ldata);
    L *= (Float)scale;
    nloJson wData = param.value("wLight", nloJson::array({1.f, 1.f, 1.f}));
    Vector3f wLight = Vector3f::fromJsonArray(wData);
    auto ret = new DistantLight(l2w, L, wLight);
    return ret;
}

REGISTER("distant", createDistantLight)

PALADIN_END
