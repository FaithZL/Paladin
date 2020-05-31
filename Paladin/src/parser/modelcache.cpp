//
//  modelcache.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/3/12.
//

#include "modelcache.hpp"
#include "tools/fileio.hpp"
#include "materials/matte.hpp"
#include "core/aggregate.hpp"
#include "lights/diffuse.hpp"
#include "tools/fileutil.hpp"
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
