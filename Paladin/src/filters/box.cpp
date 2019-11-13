//
//  box.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/14.
//

#include "box.hpp"

PALADIN_BEGIN

shared_ptr<Serializable> createBoxFilter(const neb::CJsonObject &param) {
    
    shared_ptr<Serializable> ret = make_shared<BoxFilter>(Vector2f());
    return ret;
}

REGISTER("box", createBoxFilter);

PALADIN_END
