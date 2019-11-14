//
//  gaussian.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/14.
//

#include "gaussian.hpp"

PALADIN_BEGIN

USING_STD

shared_ptr<Serializable> createGaussianFilter(const nebJson &param, initializer_list<shared_ptr<Serializable>> &) {
    nebJson radius = param.getValue("radius", "[]");
}

PALADIN_END

