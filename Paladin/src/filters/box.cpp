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
CObject_ptr createBoxFilter(const nloJson &param) {
    nloJson radius = param.value("radius", nloJson::array({2,2}));
    Float rx = radius.at(0);
    Float ry = radius.at(1);
    return new BoxFilter(Vector2f(rx, ry));
}

REGISTER("box", createBoxFilter);

PALADIN_END
