//
//  box.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/14.
//

#include "box.hpp"

PALADIN_BEGIN

/**
 * param : {
 *     "radius" : [2,2]
 * }
 */
CObject_ptr createBoxFilter(const nebJson &param) {
    nebJson radius = param.GetValue("radius", nebJson());
    Float rx = radius.GetValue(0, 2.f);
    Float ry = radius.GetValue(1, 2.f);
    return make_shared<BoxFilter>(Vector2f(rx, ry));
}

REGISTER("box", createBoxFilter);

PALADIN_END
