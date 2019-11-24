//
//  distant.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/18.
//

#include "distant.hpp"

PALADIN_BEGIN

DistantLight::DistantLight(shared_ptr<const Transform> LightToWorld, const Spectrum &L,
                           const Vector3f &wLight)
: Light((int)LightFlags::DeltaDirection, LightToWorld, MediumInterface()),
_L(L),
_wLight(normalize(LightToWorld->exec(wLight))) {
    
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

Spectrum DistantLight::power() const {
    return _L * Pi * _worldRadius * _worldRadius;
}

Float DistantLight::pdf_Li(const Interaction &, const Vector3f &) const {
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
//        "wLight" : [1,1,1]
//    }
//}
CObject_ptr createDistantLight(const nloJson &param, const Arguments &lst) {
    nloJson l2w_data = param.value("transform", nloJson());
    auto l2w = shared_ptr<const Transform>(createTransform(l2w_data));
    nloJson Ldata = param.value("L", nloJson::object());
    Spectrum L = Spectrum::FromJson(Ldata);
    nloJson wData = param.value("wLight", nloJson::array({1.f, 1.f, 1.f}));
    Vector3f wLight = Vector3f::fromJsonArray(wData);
    auto ret = new DistantLight(l2w, L, wLight);
    return ret;
}

REGISTER("distant", createDistantLight)

PALADIN_END
