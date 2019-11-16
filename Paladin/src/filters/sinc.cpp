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
CObject_ptr createSincFilter(const nebJson &param) {
	nebJson radius = param.GetValue("radius", nebJson());
    Float rx = radius.GetValue(0, 2.f);
    Float ry = radius.GetValue(1, 2.f);
    Float tau = param.GetValue("tau", 3.f);
    return new LanczosSincFilter(Vector2f(rx, ry), tau);
}

//REGISTER("sinc", createSincFilter);

PALADIN_END
