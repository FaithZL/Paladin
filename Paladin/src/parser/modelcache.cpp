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
#include "core/paladin.hpp"
#include "tools/parallel.hpp"
#include <time.h>


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
                                                          const Transform *transform,
                                                          vector<shared_ptr<Light>> &lights,
                                                          mutex * mtx) {
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
    Transform * _transform;
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
    _transform = createTransform(transformData);
    
    size_t nTriangles = _verts.size() / 3;
    * _transform = (*transform) * (*_transform);
    auto mesh = TriangleMesh::create(_transform, nTriangles, _verts,
                                     &_points, &_normals, &_UVs);
    auto w2o(_transform->getInverse_ptr());
    nloJson miData = param.value("mediumInterface", nloJson());
    MediumInterface mi = Paladin::getInstance()->getSceneParser()->getMediumInterface(miData);
    
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
//            if (mtx) {
//                std::lock_guard<std::mutex> lock(*mtx);
//                shared_ptr<const Material> pMat(createMaterial(matData));
//                prim = GeometricPrimitive::create(tri, pMat, light, mi);
//            } else {
                shared_ptr<const Material> pMat(createMaterial(matData));
                prim = GeometricPrimitive::create(tri, pMat, light, mi);
//            }
        }
        ret.push_back(prim);
    }
    
    return ret;
}

vector<shared_ptr<Primitive>> ModelCache::loadPrimitives(const string &fn,
                                                         const Transform *transform,
                                                         vector<shared_ptr<Light>> &lights) {
    
    if (hasExtension(fn, "obj")) {
        return getPrimitiveFromObj(fn, transform, lights, nullptr, false);
    }
    
    vector<shared_ptr<Primitive>> ret;
    nloJson meshList = createJsonFromFile(fn)["data"];
    for (auto meshData : meshList) {
        vector<shared_ptr<Primitive>> tmp = createPrimitive(meshData, transform, lights);
        ret.insert(ret.end(), tmp.begin(), tmp.end());
    }
    
//    size_t size = meshList.size();
//    mutex mtx;
//    parallelFor([&](int i) {
//        auto meshData = meshList[i];
//        vector<shared_ptr<Primitive>> tmp = createPrimitive(meshData, transform, lights, &mtx);
//        std::lock_guard<std::mutex> lock(mtx);
//        ret.insert(ret.end(), tmp.begin(), tmp.end());
//    }, size, 32);
    
    return ret;
}

vector<shared_ptr<Primitive>> ModelCache::getPrimitives(const string &fn,
                                                        const Transform *transform,
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
//    "materials" : [
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
vector<shared_ptr<Mesh>> ModelCache::createMeshes(const nloJson &param,
                                                const Transform *transform,
                                                vector<shared_ptr<Light>> &lights,
                                                   const shared_ptr<const Material> &mat,
                                                   const MediumInterface &mi) {
    vector<shared_ptr<Mesh>> ret;
    
    vector<Point3f> points;
    // 法线列表
    vector<Normal3f> normals;
    // uv坐标列表
    vector<Point2f> UVs;
    
    nloJson normalData = param.value("normals", nloJson::array());
    for (auto iter = normalData.cbegin(); iter != normalData.cend(); iter += 3) {
        Float nx = iter[0];
        Float ny = iter[1];
        Float nz = iter[2];
        normals.emplace_back(nx, ny, nz);
    }
    
    nloJson pointData = param.value("verts", nloJson::array());
    for (auto iter = pointData.cbegin(); iter != pointData.cend(); iter += 3) {
        Float px = iter[0];
        Float py = iter[1];
        Float pz = iter[2];
        points.emplace_back(px, py, pz);
    }
    
    nloJson UVData = param.value("UVs", nloJson::array());
    for (auto iter = UVData.cbegin(); iter != UVData.cend(); iter += 2) {
        Float u = iter[0];
        Float v = iter[1];
        UVs.emplace_back(u, v);
    }
    
    nloJson indexes = param.value("indexes", nloJson::array());
    int nMesh = indexes.size();
    
    nloJson transformData = param.value("transform", nloJson());
    Transform * transform2 = createTransform(transformData);
    
    nloJson materialDatas = param.value("materials", nloJson::array());
    
    nloJson emissionData = param.value("emission", nloJson());
    shared_ptr<const Material> material;
    if (!emissionData.is_null()) {
        material = createLightMat();
    }
    
    * transform2 = (*transform) * (*transform2);
    
    for (size_t i = 0; i < indexes.size(); ++i) {
        nloJson subIndexes = indexes[i];
        
        auto material = mat ? mat : shared_ptr<const Material>(createMaterial(materialDatas[i]));
    
        auto mesh = Mesh::create(transform2, subIndexes, &points,
                                 &normals, &UVs, material);
        
        if (!emissionData.is_null()) {
            auto al = shared_ptr<DiffuseAreaLight>(createDiffuseAreaLight(emissionData, mesh, mi));
            mesh->setAreaLight(al);
            lights.push_back(al);
        }
        
        ret.push_back(mesh);
    }
    
    return ret;
}

vector<shared_ptr<Mesh>> ModelCache::loadMeshes(const string &fn,
                                            const Transform *transform,
                                            vector<shared_ptr<Light>> &lights,
                                                 const shared_ptr<const Material> &mat,
                                                 const MediumInterface &mi) {
    vector<shared_ptr<Mesh>> ret;
    string basePath = Paladin::getInstance()->getBasePath();
    
    nloJson meshList = createJsonFromFile(basePath + fn)["data"];
    for (auto meshData : meshList) {
        vector<shared_ptr<Mesh>> tmp = createMeshes(meshData, transform, lights);
        ret.insert(ret.end(), tmp.begin(), tmp.end());
    }
    return ret;
}

vector<shared_ptr<Mesh>> ModelCache::getMeshes(const string &fn, const Transform *transform, vector<shared_ptr<Light> > &lights) {
    auto self = getInstance();
    auto iter = self->_meshMap.find(fn);
    if (iter == self->_meshMap.end()) {
        auto shapeLst = self->loadMeshes(fn, transform, lights);
        self->_meshMap[fn] = shapeLst;
        return shapeLst;
    }
    
    return self->_meshMap[fn];
}

PALADIN_END
