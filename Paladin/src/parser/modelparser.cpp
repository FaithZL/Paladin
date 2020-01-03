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
        parseMesh(shape.mesh);
    }
}

void ModelParser::parseMesh(const mesh_t &mesh) {
    for (size_t i = 0; i < mesh.indices.size(); ++i) {
        index_t idx = mesh.indices[i];
        _vertIndices.push_back(idx.vertex_index);
    }
}

void ModelParser::remedyIndices() {
    // 顶点索引与法线索引，UV索引可能不一致，需要整理
    // 以顶点索引为标准，整理
    vector<Point2f> tmpUVs(_UVs.size());
    vector<Normal3f> tmpNormals(_normals.size());
    for (size_t i = 0; i < _vertIndices.size(); ++i) {
        int pointIdx = _vertIndices[i];
        tmpNormals[i] = _normals[pointIdx];
        tmpUVs[i] = _UVs[pointIdx];
    }
    _normals.swap(tmpNormals);
    _UVs.swap(tmpUVs);
}

vector<shared_ptr<Shape>> ModelParser::getTriLst(const shared_ptr<const Transform> &o2w,
                                                 bool reverseOrientation) {
    initPoints();
    initNormals();
    initUVs();
    parseShapes();
    remedyIndices();

    auto mesh = createTriMesh(o2w, _nTriangles, &_vertIndices[0], 
                        _vertIndices.size(), _points, _UVs);

    vector<shared_ptr<Shape>> ret;
    shared_ptr<Transform> w2o(o2w->getInverse_ptr());
    // todo
    for (int i = 0; i < _nTriangles; ++i) {
        ret.push_back(createTri(o2w, w2o, reverseOrientation, mesh, i));
    }
    return ret;
}

PALADIN_END
