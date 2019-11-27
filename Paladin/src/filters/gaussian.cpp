//
//  gaussian.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/14.
//

#include "gaussian.hpp"

PALADIN_BEGIN

USING_STD

/**
 * param : {
 * 	   "radius" : [2,2],
 * 	   "alpha" : 2
 * }
 */
CObject_ptr createGaussianFilter(const nloJson &param) {
    nloJson radius = param.value("radius", nloJson::array({0.5f,0.5f}));
    Float rx = radius.at(0);
    Float ry = radius.at(1);
    Float alpha = param.value("alpha", 2.f);
    return new GaussianFilter(Vector2f(rx, ry), alpha);
}

REGISTER("gaussian", createGaussianFilter);

PALADIN_END

