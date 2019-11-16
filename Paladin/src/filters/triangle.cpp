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
CObject_ptr createTriangleFilter(const nebJson &param) {
	nebJson radius = param.GetValue("radius", nebJson());
    Float rx = radius.GetValue(0, 2.f);
    Float ry = radius.GetValue(1, 2.f);
    return make_shared<TriangleFilter>(Vector2f(rx, ry));
}

REGISTER("triangle", createTriangleFilter);

PALADIN_END
