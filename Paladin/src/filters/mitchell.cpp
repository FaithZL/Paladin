//
//  mitchell.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/14.
//

#include "mitchell.hpp"

PALADIN_BEGIN

CObject_ptr createMitchellFilter(const nebJson &param) {
	nebJson radius = param.getValue("radius", radius);
    Float rx = radius.getValue(0, 2.f);
    Float ry = radius.getValue(1, 2.f);
    Float B = param.getValue("B", 1.f/3.f);
    Float C = param.getValue("C", 1.f/3.f);
    return make_shared<MitchellFilter>(Vector2f(rx, ry), B, C);
}

REGISTER("mitchell", createMitchellFilter);

PALADIN_END
