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
#include "tools/parallel.hpp"
#include "core/medium.hpp"
#include "materials/matte.hpp"
#include "textures/constant.hpp"
#include "lights/distant.hpp"
#include "meshparser.hpp"
#include "tools/embree_util.hpp"

PALADIN_BEGIN

USING_STD;

void SceneParser::parse(const nloJson &data) {
    
    int threadNum = data.value("threadNum", 0);
    parallelInit(threadNum);
    
    nloJson filterData = data.value("filter", nloJson());
    Filter * filter = parseFilter(filterData);
    
    nloJson filmData = data.value("film", nloJson());
    auto film = parseFilm(filmData, filter);
    
    nloJson samplerData = data.value("sampler", nloJson());
    Sampler * sampler = parseSampler(samplerData, film);
    
    nloJson mediumDataDict = data.value("mediums", nloJson::object());
    parseMediums(mediumDataDict);
    
    nloJson cameraData = data.value("camera", nloJson());
    Camera * camera = parseCamera(cameraData, film);
    
    nloJson integratorData = data.value("integrator", nloJson());
    Integrator * integrator = parseIntegrator(integratorData, sampler, camera);
    _integrator.reset(integrator);
    
    nloJson materialDataDict = data.value("materials", nloJson::object());
    parseMaterials(materialDataDict);
    
    nloJson transformDict = data.value("transforms", nloJson::object());
    parseTransformMap(transformDict);

    nloJson shapesData = data.value("shapes", nloJson());
    parseShapes(shapesData);
    
    nloJson lightDataList = data.value("lights", nloJson::array());
    parseLights(lightDataList);
    
    bool autolight = data.value("autolight", false);
    if (autolight) {
        autoLight();
    }
    bool autoplane = data.value("autoplane", false);
    if (autoplane) {
        autoPlane();
    }
    
    nloJson acceleratorData = data.value("accelerator", nloJson::object());
    Scene * scene = new Scene(_lights);
    scene->initAccel(acceleratorData, _primitives);
    
    _scene.reset(scene);
    
    _integrator->render(*scene);
    
}

void SceneParser::autoPlane() {
    AABB3f bound = getPrimsBound();
    std::cout << "scene bound box is:" << bound << endl;
    auto d = bound.diagonal();
    float width = std::max(d.x, d.z);
    width *= 5;
    auto rotate = Transform::rotateX(-90);
    auto translate = Transform::translate_ptr(Vector3f(0, bound.pMin.y, 0));
    *translate = (*translate) * rotate;
    auto tex = make_shared<ConstantTexture<Spectrum>>(Spectrum(1.f));
    auto sigma = make_shared<ConstantTexture<Float>>(0);
    auto mat = make_shared<MatteMaterial>(tex, sigma, nullptr);
    auto quad = createQuad(translate, false, width);
    auto prims = createPrimitive(quad, _lights, mat, nullptr, nloJson());
    _primitives.insert(_primitives.end(), prims.begin(), prims.end());
}

void SceneParser::autoLight() {
    auto tf = Transform::identity_ptr();
    Spectrum L(1.f);
    auto light = make_shared<DistantLight>(tf, L, Vector3f(1,1,-1));
    _lights.push_back(light);
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

void SceneParser::parseMediums(const nloJson &dict) {
    for (auto iter = dict.cbegin(); iter != dict.cend(); ++iter) {
        string name = iter.key();
        nloJson data = iter.value();
        shared_ptr<const Medium> medium(createMedium(data));
        addMediumToCache(name, medium);
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

void SceneParser::parseTransformMap(const nloJson &dict) {
    for (auto iter = dict.cbegin(); iter != dict.cend(); ++iter) {
        string name = iter.key();
        nloJson data = iter.value();
        Transform * tf = createTransform(data);
        addTransformToCache(name, tf);
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
//    "type" : "pt",
//    "param" : {
//        "maxBounce" : 5,
//        "rrThreshold" : 1,
//        "lightSampleStrategy" : "power"
//    }
//}
Integrator * SceneParser::parseIntegrator(const nloJson &data, Sampler * sampler, Camera * camera) {
    string type = data.value("type", "pt");
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

//data : {
//    "type" : "triMesh",
//    "subType" : "quad",
//    "name" : "front",
//    "enable" : true,
//    "param" : {
//        "transform" :[
//            {
//                "type" : "translate",
//                "param" : [-1,0,0]
//            }
//        ],
//        "width" : 2
//    },
//    "mediumInterface" : [null, "fog"],
//    "material" : null
//}
void SceneParser::parseMesh(const nloJson &data) {
    string subType = data.value("subType", "");
    vector<shared_ptr<Primitive>> prims;
    nloJson medIntfceData = data.value("mediumInterface", nloJson());
    MediumInterface mediumInterface = getMediumInterface(medIntfceData);
    if (subType == "quad") {
        
    }
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
        } else if (type == "clonal") {
            parseClonal(shapeData);
        } else {
            parseSimpleShape(shapeData, type);
        }
    }
}

//data : {
//    "type" : "clonal",
//    "subType" : "cube",
//    "name" : "cube3",
//    "enable" : true,
//    "transform" :[
//        {
//            "type" : "rotateY",
//            "param" : [15]
//        },
//        {
//            "type" : "translate",
//            "param" : [0.35,-0.7,-0.4]
//        }
//    ],
//    "mediumInterface" : [null, "fog"],
//    "from" : "cube1",
//    "material" : "glass"
//},
void SceneParser::parseClonal(const nloJson &data) {
    // paladin的实例化暂时不支持光源
    nloJson param = data.value("param", nloJson::object());
    nloJson medIntfceData = data.value("mediumInterface", nloJson());
    MediumInterface mediumInterface = getMediumInterface(medIntfceData);
    auto mat = getMaterial(data.value("material", nloJson()));
    nloJson from = data.value("from", nloJson());
    if (from.is_null()) {
        return;
    }
    const vector<shared_ptr<Primitive>> & prims = getPrimitives(from);
    vector<shared_ptr<Primitive>> tPrims;
    auto l2w = createTransform(data.value("transform", nloJson()));
    for (auto iter = prims.cbegin(); iter != prims.cend(); ++iter) {
        auto tPrim = TransformedPrimitive::create(*iter, l2w, mat, mediumInterface);
        _primitives.push_back(tPrim);
    };
}

//"data" : {
//    "type" : "sphere",
//    "name" : "sphere1"
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
//    "mediumInterface" : [null, "fog"],
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
    nloJson matData = data.value("material", nloJson());
    shared_ptr<const Material> mat;
        
    if (matData.is_object()) {
        mat.reset(createMaterial(matData));
    } else {
        mat = getMaterial(matData);
    }
    
    nloJson medIntfceData = data.value("mediumInterface", nloJson());
    MediumInterface mediumInterface = getMediumInterface(medIntfceData);
    auto tmpLight = createDiffuseAreaLight(data.value("emission", nloJson()), shape, mediumInterface);
    shared_ptr<AreaLight> areaLight(tmpLight);
    if (areaLight) {
        _lights.push_back(areaLight);
    }
    
    shared_ptr<Primitive> primitives = GeometricPrimitive::create(shape, mat, areaLight, mediumInterface);
    _primitives.push_back(primitives);
    // 如果需要克隆的话，则保存在_cloneMap中
    if (data.value("clone", false)) {
        string name = data.value("name", "");
        vector<shared_ptr<Primitive>> v;
        v.push_back(primitives);
        addPrimitivesToCloneMap(name, v);
    }
}

//data : {
//    "type" : "triMesh",
//    "subType" : "quad",
//    "name" : "front",
//    "enable" : true,
//    "param" : {
//        "transform" :[
//            {
//                "type" : "translate",
//                "param" : [-1,0,0]
//            }
//        ],
//        "width" : 2
//    },
//    "mediumInterface" : [null, "fog"],
//    "material" : null
//}
void SceneParser::parseTriMesh(const nloJson &data) {
    string subType = data.value("subType", "");
    vector<shared_ptr<Primitive>> prims;
    nloJson medIntfceData = data.value("mediumInterface", nloJson());
    MediumInterface mediumInterface = getMediumInterface(medIntfceData);
    shared_ptr<const Material> mat = getMaterial(data.value("material", nloJson()));
    if (subType == "quad") {
        prims = createQuadPrimitive(data, mat, _lights, mediumInterface);
    } else if (subType == "cube") {
        prims = createCubePrimitive(data, mat, _lights, mediumInterface);
    } else if (subType == "model") {
        prims = createModelPrimitive(data, mat, _lights, mediumInterface);
    } else if (subType == "mesh") {
        MeshParser mp;
        prims = mp.getPrimitiveLst(data, _lights);
    }
    if (data.value("clone", false)) {
        string name = data.value("name", "");
        addPrimitivesToCloneMap(name, prims);
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
