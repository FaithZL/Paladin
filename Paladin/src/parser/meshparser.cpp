//
//  meshparser.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/3/1.
//

#include "meshparser.hpp"

PALADIN_BEGIN

//data : {
//    "type" : "triMesh",
//    "subType" : "mesh",
//    "param" : {
//        "normals" : [
//            1,0,0,
//            2,0,0
//        ],
//        "verts" : [
//            2,1,1,
//            3,2,1
//        ],
//        "UVs" : [
//            0.9,0.3,
//            0.5,0.6
//        ],
//        "material" : {
//
//        },
//        "indexes" : [
//            1,2,3,
//            3,5,6
//        ],
//        "transform" : [
//
//        ]
//    }
//},
void TriangleParser::load(const nloJson &data) {
    nloJson param = data.value("param", nloJson());
    nloJson normals = param.value("normals", nloJson::array());
    for (auto iter = normals.cbegin(); iter != normals.cend(); iter += 3) {
        Float nx = iter[0];
        Float ny = iter[1];
        Float nz = iter[2];
        _normals.emplace_back(nx, ny, nz);
    }
    
    nloJson verts = param.value("verts", nloJson::array());
    for (auto iter = verts.cbegin(); iter != verts.cend(); iter += 3) {
        Float px = iter[0];
        Float py = iter[1];
        Float pz = iter[2];
        _points.emplace_back(px, py, pz);
    }
    
    nloJson UVs = param.value("UVs", nloJson::array());
    for (auto iter = UVs.cbegin(); iter != UVs.cend(); iter += 2) {
        Float u = iter[0];
        Float v = iter[1];
        _UVs.emplace_back(u, v);
    }
    
    nloJson indexes = param.value("indexes", nloJson::array());
    for (auto iter = indexes.cbegin(); iter != indexes.cend(); ++iter) {
        _verts.emplace_back(*iter);
    }
    
    
}

vector<shared_ptr<Primitive>> TriangleParser::getPrimitiveLst() {
    return vector<shared_ptr<Primitive>>();
}

PALADIN_END
