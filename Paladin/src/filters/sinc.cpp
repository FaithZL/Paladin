//
//  sinc.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/14.
//

#include "sinc.hpp"

PALADIN_BEGIN

USING_STD

shared_ptr<Serializable> createSincFilter(const nebJson &param) {
	nebJson radius = param.getValue("radius", radius);
    Float rx = radius.getValue(0, 2.f);
    Float ry = radius.getValue(1, 2.f);
    Float tau = param.getValue("tau", 3.f);
    return make_shared<LanczosSincFilter>(Vector2f(rx, ry), tau);
}

//REGISTER("sinc", createSincFilter);

PALADIN_END
