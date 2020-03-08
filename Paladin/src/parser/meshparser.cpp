//
//  meshparser.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/3/1.
//

#include "meshparser.hpp"
#include "materials/matte.hpp"
#include "lights/diffuse.hpp"
#include "core/primitive.hpp"

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
//            "type" : "unity",
//            "param" : {
//                "albedo" : [0.725, 0.71, 0.68],
//                "roughness" : 0.2,
//                "metallic" : 0.8
//            }
//        },
//        "indexes" : [
//            1,2,3,
//            3,5,6
//        ],
//        "transform" : {
//            "type" : "matrix",
//            "param" : [
//                1,0,0,0,
//                0,1,0,0,
//                0,0,1,0,
//                0,0,0,1
//            ]
//        }
//    }
//    "emission" : {
//        "nSamples" : 1,
//        "Le" : {
//            "colorType" : 1,
//            "color" : [1,1,1]
//        },
//        "twoSided" : false
//    }
//},
void MeshParser::load(const nloJson &data) {
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
    
    _emissionData = data.value("emission", nloJson());
    if (_emissionData.is_null()) {
        nloJson matData = param.value("material", nloJson());
        _material.reset(createMaterial(matData));
    } else {
        _material = createLightMat();
    }
    
    nloJson transformData = param.value("transform", nloJson());
    _transform.reset(createTransform(transformData));
}

vector<shared_ptr<Primitive>> MeshParser::getPrimitiveLst(vector<shared_ptr<Light>> &lights) {
    vector<shared_ptr<Primitive>> ret;
    
    size_t nTriangles = _verts.size() / 3;
    
    auto mesh = TriangleMesh::create(_transform, nTriangles, _verts,
                                     &_points, &_normals, &_UVs);
    shared_ptr<Transform> w2o(_transform->getInverse_ptr());
    vector<shared_ptr<Shape>> triLst;
    MediumInterface mi(nullptr);
    for (size_t i = 0; i < nTriangles; ++i) {
        auto tri = createTri(_transform, w2o, false, mesh, i);
        shared_ptr<DiffuseAreaLight> light;
        shared_ptr<GeometricPrimitive> prim;
        if (!_emissionData.is_null()) {
            light.reset(createDiffuseAreaLight(_emissionData, tri, mi));
            prim = GeometricPrimitive::create(tri, _material, light, mi);
            lights.push_back(light);
        } else {
            prim = GeometricPrimitive::create(tri, nullptr, nullptr, mi);
        }
        ret.push_back(prim);
    }
    return ret;
}

PALADIN_END
