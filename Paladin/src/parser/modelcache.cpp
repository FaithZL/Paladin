//
//  modelcache.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/3/12.
//

#include "modelcache.hpp"
#include "tools/fileio.hpp"
#include "materials/matte.hpp"
#include "core/primitive.hpp"
#include "lights/diffuse.hpp"
#include "tools/fileutil.hpp"
#include "shapes/trianglemesh.hpp"

PALADIN_BEGIN

ModelCache * ModelCache::s_modelCache = nullptr;

ModelCache * ModelCache::getInstance() {
    if (s_modelCache == nullptr) {
        s_modelCache = new ModelCache();
    }
    return s_modelCache;
}

//"param" : {
//    "normals" : [
//        1,0,0,
//        2,0,0
//    ],
//    "verts" : [
//        2,1,1,
//        3,2,1
//    ],
//    "UVs" : [
//        0.9,0.3,
//        0.5,0.6
//    ],
//    "indexes" : [
//        [1,2,3],
//        [3,5,6]
//    ],
//    "material" : [
//        {
//            "type" : "unity",
//            "param" : {
//                "albedo" : [0.725, 0.71, 0.68],
//                "roughness" : 0.2,
//                "metallic" : 0.8
//            }
//        },
//        {
//            "type" : "unity",
//            "param" : {
//                "albedo" : [0.725, 0.71, 0.68],
//                "roughness" : 0.2,
//                "metallic" : 0.8
//            }
//        }
//    ],
//    "transform" : {
//        "type" : "matrix",
//        "param" : [
//            1,0,0,0,
//            0,1,0,0,
//            0,0,1,0,
//            0,0,0,1
//        ]
//    },
//    "emission" : {
//        "nSamples" : 1,
//        "Le" : {
//            "colorType" : 1,
//            "color" : [1,1,1]
//        },
//        "twoSided" : false
//    }
//},
vector<shared_ptr<Primitive>> ModelCache::createPrimitive(const nloJson &param,
                                                          const shared_ptr<const Transform> &transform,
                                                          vector<shared_ptr<Light>> &lights) {
    vector<shared_ptr<Primitive>> ret;
    
    vector<Point3f> _points;
    // 法线列表
    vector<Normal3f> _normals;
    // uv坐标列表
    vector<Point2f> _UVs;
    // 顶点索引列表
    vector<Index> _verts;

    vector<int> startIdxs;
    
    // 发光参数
    nloJson _emissionData;
    shared_ptr<Transform> _transform;
    shared_ptr<const Material> _material;
    vector<shared_ptr<const Material>> _materials;
    
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
    int counter = 0;
    for (auto iter = indexes.cbegin(); iter != indexes.cend(); ++iter) {
        nloJson subIndexes = *iter;
        
        for(auto _iter = subIndexes.cbegin(); _iter != subIndexes.cend(); ++_iter) {
            _verts.emplace_back(*_iter);
            ++counter;
        }
        startIdxs.push_back(counter);
    }
    
    _emissionData = param.value("emission", nloJson());
    if (!_emissionData.is_null()) {
        _material = createLightMat();
    }
    
    nloJson transformData = param.value("transform", nloJson());
    _transform.reset(createTransform(transformData));
    
    size_t nTriangles = _verts.size() / 3;
    * _transform = (*transform) * (*_transform);
    auto mesh = TriangleMesh::create(_transform, nTriangles, _verts,
                                     &_points, &_normals, &_UVs);
    shared_ptr<Transform> w2o(_transform->getInverse_ptr());
    MediumInterface mi(nullptr);
    
    int matIdx = 0;
    nloJson matLst = param.value("materials", nloJson());
    for (size_t i = 0; i < nTriangles; ++i) {
        auto tri = createTri(_transform, w2o, false, mesh, i);
        shared_ptr<DiffuseAreaLight> light;
        shared_ptr<GeometricPrimitive> prim;
        if (!_emissionData.is_null()) {
            light.reset(createDiffuseAreaLight(_emissionData, tri, mi));
            prim = GeometricPrimitive::create(tri, _material, light, mi);
            lights.push_back(light);
        } else {
            int vertIdx = i * 3;
            if (vertIdx > startIdxs[matIdx]) {
                ++matIdx;
            }
            nloJson matData = matLst[matIdx];
            nloJson emissionData = matData.value("emission", nloJson());
            if (!emissionData.is_null()) {
                light.reset(createDiffuseAreaLight(_emissionData, tri, mi));
            }
            shared_ptr<const Material> pMat(createMaterial(matData));
            prim = GeometricPrimitive::create(tri, pMat, light, mi);
        }
        ret.push_back(prim);
    }
    
    return ret;
}

vector<shared_ptr<Primitive>> ModelCache::loadPrimitives(const string &fn,
                                                         const shared_ptr< Transform> &transform,
                                                         vector<shared_ptr<Light>> &lights) {
    
    if (hasExtension(fn, "obj")) {
        Transform swapHand = Transform::scale(-1, 1, 1);
        *transform = (*transform) * swapHand;
        return getPrimitiveFromObj(fn, transform, lights, nullptr, false);
    }
    
    vector<shared_ptr<Primitive>> ret;
    nloJson meshList = createJsonFromFile(fn);
    for(const nloJson &meshData : meshList) {
        vector<shared_ptr<Primitive>> tmp = createPrimitive(meshData, transform, lights);
        ret.insert(ret.end(), tmp.begin(), tmp.end());
    }
    
    return ret;
}

vector<shared_ptr<Primitive>> ModelCache::getPrimitives(const string &fn,
                                                        const shared_ptr<Transform> &transform,
                                                        vector<shared_ptr<Light>> &lights) {
    auto iter = _modelMap.find(fn);
    if (iter == _modelMap.end()) {
        auto primLst = loadPrimitives(fn, transform, lights);
        _modelMap[fn] = primLst;
        return primLst;
    }
    vector<shared_ptr<Primitive>> tmpLst = _modelMap[fn];
    vector<shared_ptr<Primitive>> ret;
    ret.reserve(tmpLst.size());
    for (auto iter = tmpLst.cbegin(); iter != tmpLst.cend(); ++iter) {
        auto transformPrim = TransformedPrimitive::create(*iter, transform);
        ret.push_back(transformPrim);
    }
    return ret;
}

PALADIN_END
