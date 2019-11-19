//
//  triangle.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/14.
//

#include "triangle.hpp"
#include "tools/classfactory.hpp"

PALADIN_BEGIN

/**
 * param : {
 * 	   "radius" : [2,2]
 * }
 */
CObject_ptr createTriangleFilter(const nloJson &param) {
    nloJson radius = param.value("radius", nloJson::array({2,2}));
    Float rx = radius.at(0);
    Float ry = radius.at(1);
    return new TriangleFilter(Vector2f(rx, ry));
}

REGISTER("triangle", createTriangleFilter);

PALADIN_END
