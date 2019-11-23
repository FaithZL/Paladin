//
//  pointlight.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "pointlight.hpp"
#include "math/sampling.hpp"

PALADIN_BEGIN

Spectrum PointLight::sample_Li(const Interaction &ref, const Point2f &u,
                               Vector3f *wi, Float *pdf,
                               VisibilityTester *vis) const {
    *wi = normalize(_pos - ref.pos);
    // 点光源比较特殊
    // ref点采样点光源时，只有一个方向，就是wi
    // 为了统一MC积分的格式：f(x)/p(x)，所以把p(x)赋值为1，
    // 这个问题想象起来可能比较抽象
    *pdf = 1.0f;
    *vis = VisibilityTester(ref, Interaction(_pos, ref.time, mediumInterface));
    return _I / distanceSquared(_pos, ref.pos);
}

// 狄拉克函数
// 特殊处理
Float PointLight::pdf_Li(const Interaction &, const Vector3f &) const {
    return 0;
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
//}
CObject_ptr createPointLight(const nloJson &param, const Arguments &lst) {
    nloJson l2w_data = param.value("lightToWorld", nloJson());
    auto l2w = shared_ptr<const Transform>(createTransform(l2w_data));
    nloJson Idata = param.value("I", nloJson::object());
    Spectrum I = Spectrum::FromJson(Idata);
    auto ret = new PointLight(l2w, nullptr, I);
    return ret;
}

REGISTER("pointLight", createPointLight)

PALADIN_END
