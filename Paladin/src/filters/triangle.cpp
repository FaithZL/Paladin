//
//  triangle.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/14.
//

#include "triangle.hpp"
#include "tools/classfactory.hpp"

PALADIN_BEGIN

CObject_ptr createTriangleFilter(const neb::CJsonObject &param) {
	nebJson radius = param.getValue("radius", radius);
    Float rx = radius.getValue(0, 2.f);
    Float ry = radius.getValue(1, 2.f);
    return make_shared<TriangleFilter>(Vector2f(rx, ry));
}

REGISTER("triangle", createTriangleFilter);

PALADIN_END
