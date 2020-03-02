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
//        "index" : [
//            1,2,3,
//            3,5,6
//        ],
//        "transform" : [
//
//        ]
//    }
//},
void TriangleParser::load(const nloJson &data) {
    
}

vector<shared_ptr<Primitive>> TriangleParser::getPrimitiveLst() {
    return vector<shared_ptr<Primitive>>();
}

PALADIN_END
