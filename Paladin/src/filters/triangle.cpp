//
//  triangle.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/14.
//

#include "triangle.hpp"

PALADIN_BEGIN

shared_ptr<Serializable> createTriangleFilter(const neb::CJsonObject &) {
	nebJson radius = param.getValue("radius", radius);
    Float rx = radius.getValue(0, 2.f);
    Float ry = radius.getValue(1, 2.f);
    shared_ptr<Serializable> ret = make_shared<TriangleFilter>(Vector2f(rx, ry));
    return ret;
}

REGISTER("triangle", createTriangleFilter);

PALADIN_END
