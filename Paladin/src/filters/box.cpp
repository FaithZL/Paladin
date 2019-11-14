//
//  box.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/14.
//

#include "box.hpp"

PALADIN_BEGIN

shared_ptr<Serializable> createBoxFilter(const nebJson &param) {
    nebJson radius = param.getValue("radius", radius);
    Float rx = radius.getValue(0, 2.f);
    Float ry = radius.getValue(1, 2.f);
    shared_ptr<Serializable> ret = make_shared<BoxFilter>(Vector2f(rx, ry));
    return ret;
}

REGISTER("box", createBoxFilter);

PALADIN_END
