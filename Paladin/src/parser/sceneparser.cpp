//
//  sceneparser.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/11.
//

#include "sceneparser.hpp"
#include "tools/classfactory.hpp"
#include "core/film.hpp"
#include "tools/classfactory.hpp"
#include "filters/box.hpp"
#include "core/camera.hpp"
#include "core/shape.hpp"
#include "core/material.hpp"
#include "core/light.hpp"
#include "lights/diffuse.hpp"
#include "shapes/trianglemesh.hpp"

PALADIN_BEGIN

USING_STD;

void SceneParser::parse(const nloJson &data) {
    nloJson filterData = data.value("filter", nloJson());
    Filter * filter = parseFilter(filterData);
    
    nloJson filmData = data.value("film", nloJson());
    auto film = parseFilm(filmData, filter);
    
    nloJson samplerData = data.value("sampler", nloJson());
    Sampler * sampler = parseSampler(samplerData, film);
    
    nloJson cameraData = data.value("camera", nloJson());
    Camera * camera = parseCamera(cameraData, film);
    
    nloJson integratorData = data.value("integrator", nloJson());
    Integrator * integrator = parseIntegrator(integratorData, sampler, camera);
    _integrator.reset(integrator);
    
    nloJson materialDataList = data.value("materials", nloJson::object());
    parseMaterials(materialDataList);

    nloJson shapesData = data.value("shapes", nloJson());
    parseShapes(shapesData);
    
    nloJson lightDataList = data.value("lights", nloJson::array());
    parseLights(lightDataList);
    
    nloJson acceleratorData = data.value("accelerator", nloJson::object());
    _aggregate = parseAccelerator(acceleratorData);
    
    auto scene = new Scene(_aggregate, _lights);
//    _scene.reset(scene);
    
    _integrator->render(*scene);
    
}

void SceneParser::parseLights(const nloJson &list) {
    for (auto iter = list.cbegin(); iter != list.cend(); ++iter) {
        nloJson data = *iter;
        shared_ptr<Light> light(createLight(data));
        if (light) {
            _lights.push_back(light);
        }
    }
}

void SceneParser::parseMaterials(const nloJson &dict) {
    for (auto iter = dict.cbegin(); iter != dict.cend(); ++iter) {
        string name = iter.key();
        nloJson data = iter.value();
        shared_ptr<const Material> material(createMaterial(data));
        addMaterialToCache(name, material);
    }
}

//"sampler" : {
//    "type" : "stratified",
//    "param" : {
//        "xSamples" : 2,
//        "ySamples" : 2,
//        "jitter" : true,
//        "dimensions" : 10
//    }
//},
Sampler * SceneParser::parseSampler(const nloJson &data, Film * film) {
    string samplerType = data.value("type", "stratified");
    nloJson param = data.value("param", nloJson());
    auto creator = GET_CREATOR(samplerType);
    auto ret = dynamic_cast<Sampler *>(creator(param, {film}));
    return ret;
}

//"camera" : {
//    "type" : "perspective",
//    "param" : {
//        "shutterOpen" : 0,
//        "shutterClose" : 1,
//        "lensRadius" : 0,
//        "focalDistance" : 100,
//        "fov" : 45,
//        "lookAt" : [
//            [0,0,-5],
//            [0,0,0],
//            [0,1,0]
//        ],
//        "lookAtEnd" : [
//            [0,0,-5],
//            [0,0,0],
//            [0,1,0]
//        ]
//    }
//},
Camera * SceneParser::parseCamera(const nloJson &data, Film * film) {
    string cameraType = data.value("type", "perspective");
    nloJson param = data.value("param", nloJson());
    auto creator = GET_CREATOR(cameraType);
    auto ret = dynamic_cast<Camera *>(creator(param, {film}));
    return ret;
}

//"integrator" : {
//    "type" : "PathTracer",
//    "param" : {
//        "maxBounce" : 5,
//        "rrThreshold" : 1,
//        "lightSampleStrategy" : "power"
//    }
//}
Integrator * SceneParser::parseIntegrator(const nloJson &data, Sampler * sampler, Camera * camera) {
    string type = data.value("type", "PathTracer");
    nloJson param = data.value("param", nloJson());
    auto creator = GET_CREATOR(type);
    auto ret = dynamic_cast<Integrator *>(creator(param,{sampler, camera}));
    return ret;
}

//"filter" : {
//    "type" : "box",
//    "param" : {
//        "radius" : [2,2]
//    }
//}
Filter * SceneParser::parseFilter(const nloJson &data) {
    string filterType = data.value("type", "box");
    nloJson param = data.value("param", nloJson());
    auto creator = GET_CREATOR(filterType.c_str());
    auto ret = dynamic_cast<Filter *>(creator(param, {}));
    DCHECK(ret != nullptr);
    return ret;
}

void SceneParser::parseShapes(const nloJson &shapeDataList) {
    for (const auto &shapeData : shapeDataList) {
        string type = shapeData.value("type", "sphere");
        bool enable = shapeData.value("enable", true);
        if (!enable) {
            continue;
        }
        if (type == "triMesh") {
            parseTriMesh(shapeData);
        } else {
            parseSimpleShape(shapeData, type);
        }
    }
}

//"data" : {
//    "type" : "sphere",
//    "param" : {
//        "worldToLocal" : {
//            "type" : "translate",
//            "param" : [1,0,1]
//        },
//        "radius" : 0.75,
//        "zMin" : 0.75,
//        "zMax" : -0.75,
//        "phiMax" : 360
//    },
//    "material" : "matte1" ,
//    "emission" : {
//        "nSamples" : 1,
//        "Le" : {
//            "colorType" : 1,
//            "color" : [1,1,1]
//        },
//        "twoSided" : false
//    }
//}
void SceneParser::parseSimpleShape(const nloJson &data, const string &type) {
    nloJson param = data.value("param", nloJson::object());
    auto creator = GET_CREATOR(type);
    auto shape = shared_ptr<Shape>(dynamic_cast<Shape *>(creator(param, {})));
    auto mat = getMaterial(data.value("material", nloJson()));
    auto tmpLight = createDiffuseAreaLight(data.value("emission", nloJson()), shape);
    shared_ptr<AreaLight> areaLight(tmpLight);
    if (areaLight) {
        _lights.push_back(areaLight);
    }
    shared_ptr<Primitive> primitives = GeometricPrimitive::create(shape, mat, areaLight, nullptr);
    _primitives.push_back(primitives);
}

void SceneParser::parseTriMesh(const nloJson &data) {
    string subType = data.value("subType", "");
    vector<shared_ptr<Primitive>> prims;
    if (subType == "quad") {
        shared_ptr<const Material> mat = getMaterial(data.value("material", nloJson()));
        prims = createQuadPrimitive(data, mat, _lights);
    } else if (subType == "cube") {
        shared_ptr<const Material> mat = getMaterial(data.value("material", nloJson()));
        prims = createCubePrimitive(data, mat, _lights);
    }
    _primitives.insert(_primitives.end(), prims.begin(), prims.end());
}

//"data" : {
//    "type" : "bvh",
//    "param" : {
//        "maxPrimsInNode" : 1,
//        "splitMethod" : "SAH"
//    }
//}
shared_ptr<Aggregate> SceneParser::parseAccelerator(const nloJson &data) {
    return createAccelerator(data, _primitives);
}

Film * SceneParser::parseFilm(const nloJson &data, Filter * filt) {
    nloJson param = data.value("param", nloJson());
    Film * film = dynamic_cast<Film *>(createFilm(param,{filt}));
    return film;
}

PALADIN_END
