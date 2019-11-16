//
//  box.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/14.
//

#include "box.hpp"

PALADIN_BEGIN

CObject_ptr createBoxFilter(const nebJson &param) {
    nebJson radius = param.getValue("radius", radius);
    Float rx = radius.getValue(0, 2.f);
    Float ry = radius.getValue(1, 2.f);
    return make_shared<BoxFilter>(Vector2f(rx, ry));
}

REGISTER("box", createBoxFilter);

PALADIN_END
