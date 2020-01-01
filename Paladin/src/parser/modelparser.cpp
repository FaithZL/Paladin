//
//  modelparser.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/1/1.
//

#include "modelparser.hpp"

PALADIN_BEGIN

void ModelParser::load(const string &fn, const string &basePath, bool triangulate) {
    string warn;
    string err;
    tinyobj::LoadObj(&_attrib, &_shapes, &_materials,
                     &warn, &err, fn.c_str(),
                     nullptr, triangulate);
    
    return;
}

PALADIN_END
