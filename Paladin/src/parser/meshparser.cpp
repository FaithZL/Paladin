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
#include "core/paladin.hpp"
#include "modelcache.hpp"

PALADIN_BEGIN


// data : {
//     "type" : "triMesh",
//     "subType" : "mesh",
//     "param" : {
//         "normals" : [
//             1,0,0,
//             2,0,0
//         ],
//         "verts" : [
//             2,1,1,
//             3,2,1
//         ],
//         "UVs" : [
//             0.9,0.3,
//             0.5,0.6
//         ],
//         "indexes" : [
//             1,2,3,
//             3,5,6
//         ],
//         "material" : {
//             "type" : "unity",
//             "param" : {
//                 "albedo" : [0.725, 0.71, 0.68],
//                 "roughness" : 0.2,
//                 "metallic" : 0.8
//             }
//         },
//         "transform" : {
//             "type" : "matrix",
//             "param" : [
//                 1,0,0,0,
//                 0,1,0,0,
//                 0,0,1,0,
//                 0,0,0,1
//             ]
//         },
//         "emission" : {
//             "nSamples" : 1,
//             "Le" : {
//                 "colorType" : 1,
//                 "color" : [1,1,1]
//             },
//             "twoSided" : false
//         }
//     },
//     "transform" : {
//         "type" : "matrix",
//         "param" : [
//             1,0,0,0,
//             0,1,0,0,
//             0,0,1,0,
//             0,0,0,1
//         ]
//     },
// },
vector<shared_ptr<Primitive>> MeshParser::getPrimitiveLst(const nloJson &data,
                                                          vector<shared_ptr<Light>> &lights) {
    nloJson param = data.value("param", nloJson());
    
    nloJson transformData = data.value("transform", nloJson());
    shared_ptr<Transform> transform(createTransform(transformData));
    
    if (param.is_string()) {
        string fn = param;
        fn = Paladin::getInstance()->getBasePath() + fn;
        return ModelCache::getInstance()->getPrimitives(fn, transform, lights);
    }
    
    return getPrimitiveLst(param, lights, transform);
}

vector<shared_ptr<Primitive>> MeshParser::getPrimitiveLst(const nloJson &param,
                                            vector<shared_ptr<Light>> &lights,
                                            const shared_ptr<Transform> &transform) {
    vector<shared_ptr<Primitive>> ret;
    
    vector<Point3f> _points;
    // 法线列表
    vector<Normal3f> _normals;
    // uv坐标列表
    vector<Point2f> _UVs;
    // 顶点索引列表
    vector<Index> _verts;
    // 发光参数
    nloJson _emissionData;
    shared_ptr<Transform> _transform;
    shared_ptr<const Material> _material;
    
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
    
    _emissionData = param.value("emission", nloJson());
    if (_emissionData.is_null()) {
        nloJson matData = param.value("material", nloJson());
        _material.reset(createMaterial(matData));
    } else {
        _material = createLightMat();
    }
    
    nloJson transformData2 = param.value("transform", nloJson());
    _transform.reset(createTransform(transformData2));
    
    size_t nTriangles = _verts.size() / 3;
    
    * _transform = (*transform) * (*_transform);
    
    auto mesh = TriangleMesh::create(_transform, nTriangles, _verts,
                                     &_points, &_normals, &_UVs);
    shared_ptr<Transform> w2o(_transform->getInverse_ptr());
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
            prim = GeometricPrimitive::create(tri, _material, nullptr, mi);
        }
        ret.push_back(prim);
    }
    
    return ret;
}


PALADIN_END
