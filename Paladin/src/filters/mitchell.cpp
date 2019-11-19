//
//  mitchell.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/14.


#include "mitchell.hpp"

PALADIN_BEGIN

/**
 * param : {
 * 	   "radius" : [2,2],
 * 	   "B" : 1/3,
 * 	   "C" : 1/3,
 * }
 */
CObject_ptr createMitchellFilter(const nloJson &param) {
    nloJson radius = param.value("radius", nloJson::array({2,2}));
    Float rx = radius.at(0);
    Float ry = radius.at(1);
    Float B = param.value("B", 1.f/3.f);
    Float C = param.value("C", 1.f/3.f);
    return new MitchellFilter(Vector2f(rx, ry), B, C);
}

REGISTER("mitchell", createMitchellFilter);

PALADIN_END
