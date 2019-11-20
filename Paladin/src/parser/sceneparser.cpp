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


PALADIN_BEGIN

USING_STD;

void SceneParser::parse(const nloJson &data) {
    nloJson filterData = data.value("filter", nloJson());
    Filter * filter = parseFilter(filterData);
    
    nloJson samplerData = data.value("sampler", nloJson());
    Sampler * sampler = parseSampler(samplerData);
    
    nloJson filmData = data.value("film", nloJson());
    auto film = parseFilm(filmData, filter);
    nloJson cameraData = data.value("camera", nloJson());
    Camera * camera = parseCamera(cameraData, film);
    
    nloJson integratorData = data.value("integrator", nloJson());
    Integrator * integrator = parseIntegrator(integratorData, sampler, camera);
    _integrator.reset(integrator);

    nloJson shapesData = data.value("shapes", nloJson());
    parseShapes(shapesData);
    
    nloJson textureDataList = data.value("textures", nloJson::object());
    parseTextures(textureDataList);
    
    nloJson materialDataList = data.value("materials", nloJson::object());
    parseMaterials(materialDataList);
}

void SceneParser::parseTextures(const nloJson &list) {
    cout << setw(4) << list;
}

void SceneParser::parseMaterials(const nloJson &list) {
    cout << setw(4) << list;
}

Sampler * SceneParser::parseSampler(const nloJson &data) {
    string samplerType = data.value("type", "stratified");
    nloJson param = data.value("param", nloJson());
    auto creator = GET_CREATOR(samplerType);
    auto ret = dynamic_cast<Sampler *>(creator(param, {}));
    return ret;
}

Camera * SceneParser::parseCamera(const nloJson &data, Film * film) {
    string cameraType = data.value("type", "perspective");
    nloJson param = data.value("param", nloJson());
    auto creator = GET_CREATOR(cameraType);
    auto ret = dynamic_cast<Camera *>(creator(param, {film}));
    return ret;
}

Integrator * SceneParser::parseIntegrator(const nloJson &data, Sampler * sampler, Camera * camera) {
    string type = data.value("type", "PathTracer");
    nloJson param = data.value("param", nloJson());
    auto creator = GET_CREATOR(type);
    auto ret = dynamic_cast<Integrator *>(creator(param,{sampler, camera}));
    return ret;
}

Filter * SceneParser::parseFilter(const nloJson &data) {
    string filterType = data.value("type", "box");
    nloJson param = data.value("param", nloJson());
    auto creator = GET_CREATOR(filterType.c_str());
    auto ret = dynamic_cast<Filter *>(creator(param, {}));
    DCHECK(ret != nullptr);
    return ret;
}

void SceneParser::parseShapes(const nloJson &shapeDataList) {
    for (auto &shapeData : shapeDataList) {
        string type = shapeData.value("type", "sphere");
        if (type == "model") {
            parseModel(shapeData);
        } else {
            parseSimpleShape(shapeData, type);
        }
    }
}

void SceneParser::parseSimpleShape(const nloJson &data, const string &type) {
    nloJson param = data.value("param", nloJson());
    auto creator = GET_CREATOR(type);
    Shape * shape = dynamic_cast<Shape *>(creator(param, {}));
}

void SceneParser::parseModel(const nloJson &data) {
    
}

shared_ptr<Aggregate> SceneParser::parseAccelerator(const nloJson &param) {
    
}

Film * SceneParser::parseFilm(const nloJson &data, Filter * filt) {
    nloJson param = data.value("param", nloJson());
    Film * film = dynamic_cast<Film *>(createFilm(param,{filt}));
    return film;
}

PALADIN_END
