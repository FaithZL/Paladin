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
CObject_ptr createGaussianFilter(const nebJson &param) {
    nebJson radius = param.GetValue("radius", nebJson());
    Float rx = radius.GetValue(0, 2.f);
    Float ry = radius.GetValue(1, 2.f);
    Float alpha = param.GetValue("alpha", 2.f);
    return new GaussianFilter(Vector2f(rx, ry), alpha);
}

REGISTER("gaussian", createGaussianFilter);

PALADIN_END

