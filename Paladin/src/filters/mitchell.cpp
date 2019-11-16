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
CObject_ptr createMitchellFilter(const nebJson &param) {
	nebJson radius = param.GetValue("radius", nebJson());
    Float rx = radius.GetValue(0, 2.f);
    Float ry = radius.GetValue(1, 2.f);
    Float B = param.GetValue("B", 1.f/3.f);
    Float C = param.GetValue("C", 1.f/3.f);
    return new MitchellFilter(Vector2f(rx, ry), B, C);
}

REGISTER("mitchell", createMitchellFilter);

PALADIN_END
