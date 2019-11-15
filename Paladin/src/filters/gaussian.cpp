//
//  gaussian.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/14.
//

#include "gaussian.hpp"

PALADIN_BEGIN

USING_STD

shared_ptr<Serializable> createGaussianFilter(const nebJson &param) {
    nebJson radius = param.getValue("radius", radius);
    Float rx = radius.getValue(0, 2.f);
    Float ry = radius.getValue(1, 2.f);
    Float alpha = param.getValue("alpha", 2.f);
    return make_shared<GaussianFilter>(Vector2f(rx, ry), alpha);
}

REGISTER("gaussian", createGaussianFilter);

PALADIN_END

