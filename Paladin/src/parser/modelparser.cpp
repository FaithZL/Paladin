//
//  modelparser.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/1/1.
//

#include "modelparser.hpp"

PALADIN_BEGIN

bool ModelParser::load(const string &fn, const string &basePath, bool triangulate) {
    string warn;
    string err;
    bool ret = tinyobj::LoadObj(&_attrib, &_shapes, &_materials,
                     &warn, &err, fn.c_str(),
                     nullptr, triangulate);
    
    if (!warn.empty()) {
        std::cout << "WARN: " << warn << std::endl;
    }
    
    if (!err.empty()) {
        std::cout << "WARN: " << err << std::endl;
    }
    
    return ret;
}

void ModelParser::initPoints() {
    for (size_t i = 0; i < _attrib.vertices.size() / 3; ++i) {
        Float x = _attrib.vertices[i * 3 + 0];
        Float y = _attrib.vertices[i * 3 + 1];
        Float z = _attrib.vertices[i * 3 + 2];
        _points.emplace_back(x,y,z);
    }
}

void ModelParser::initNormals() {
    for (size_t i = 0; i < _attrib.normals.size() / 3; ++i) {
        Float x = _attrib.normals[i * 3 + 0];
        Float y = _attrib.normals[i * 3 + 1];
        Float z = _attrib.normals[i * 3 + 2];
        _normals.emplace_back(x, y, z);
    }
}

void ModelParser::initUVs() {
    for (size_t i = 0; i < _attrib.texcoords.size() / 2; ++i) {
        Float u = _attrib.texcoords[i * 2 + 0];
        Float v = _attrib.texcoords[i * 2 + 1];
        _UVs.emplace_back(u, v);
    }
}

void ModelParser::parseShapes() {
    for (size_t i = 0; i < _shapes.size(); ++i) {
        shape_t shape = _shapes.at(i);
    }
}

vector<shared_ptr<Shape>> ModelParser::getTriLst(const shared_ptr<const Transform> &o2w,
                                                 bool reverseOrientation) {
    initPoints();
    initNormals();
    initUVs();
    
    
}

PALADIN_END
