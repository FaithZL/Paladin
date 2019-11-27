//
//  sinc.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/14.
//

#include "sinc.hpp"

PALADIN_BEGIN

USING_STD

/**
 * param : {
 * 	   "radius" : [2,2],
 * 	   "tau" : 3
 * }
 */
CObject_ptr createSincFilter(const nloJson &param) {
	nloJson radius = param.value("radius", nloJson::array({0.5f,0.5f}));
    Float rx = radius.at(0);
    Float ry = radius.at(1);
    Float tau = param.value("tau", 3.f);
    return new LanczosSincFilter(Vector2f(rx, ry), tau);
}

//REGISTER("sinc", createSincFilter);

PALADIN_END
