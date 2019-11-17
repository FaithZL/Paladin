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

using namespace neb;
USING_STD;

void SceneParser::parse(const nebJson &data) {
    nebJson filterData = data.GetValue("filter", nebJson());
    Filter * filter = parseFilter(filterData);
    nebJson samplerData = data.GetValue("sampler", nebJson());
    Sampler * sampler = parseSampler(samplerData);
    nebJson filmData = data.GetValue("film", nebJson());
    auto film = parseFilm(filmData, filter);
    nebJson cameraData = data.GetValue("camera", nebJson());
    Camera * camera = parseCamera(cameraData, film);
    nebJson integratorData = data.GetValue("integrator", nebJson());
    Integrator * integrator = parseIntegrator(integratorData, sampler, camera);
    _integrator.reset(integrator);
    
    nebJson shapesData = data.GetValue("shapes", nebJson());
    parseShapes(shapesData);
}


Sampler * SceneParser::parseSampler(const nebJson &data) {
    string samplerType = data.GetValue("type", "stratified");
    nebJson param = data.GetValue("param", nebJson());
    auto creator = GET_CREATOR(samplerType);
    auto ret = dynamic_cast<Sampler *>(creator(param, {}));
    return ret;
}

Camera * SceneParser::parseCamera(const nebJson &data, Film * film) {
    string cameraType = data.GetValue("type", "perspective");
    nebJson param = data.GetValue("param", nebJson());
    auto creator = GET_CREATOR(cameraType);
    auto ret = dynamic_cast<Camera *>(creator(param, {film}));
    return ret;
}

Integrator * SceneParser::parseIntegrator(const nebJson &data, Sampler * sampler, Camera * camera) {
    string type = data.GetValue("type", "PathTracer");
    nebJson param = data.GetValue("param", nebJson());
    auto creator = GET_CREATOR(type);
    auto ret = dynamic_cast<Integrator *>(creator(param,{sampler, camera}));
    return ret;
}

Filter * SceneParser::parseFilter(const nebJson &data) {
    string filterType = data.GetValue("type", "box");
    nebJson param = data.GetValue("param", nebJson());
    auto creator = GET_CREATOR(filterType.c_str());
    auto ret = dynamic_cast<Filter *>(creator(param, {}));
    DCHECK(ret != nullptr);
    return ret;
}

void SceneParser::parseShapes(const nebJson &shapeListData) {
    int num = shapeListData.GetArraySize();
    for (int i = 0; i < num; ++i) {
        nebJson shapeData = shapeListData.GetValue(i, nebJson());
        string type = shapeData.GetValue("type", "sphere");
        if (type == "model") {
            parseModel(shapeData);
        } else {
            parseSimpleShape(shapeData, type);
        }
    }
}

void SceneParser::parseSimpleShape(const nebJson &data, const string &type) {
    nebJson param = data.GetValue("param", nebJson());
    auto creator = GET_CREATOR(type);
    Shape * shape = dynamic_cast<Shape *>(creator(param, {}));
}

void SceneParser::parseModel(const nebJson &data) {
    
}

shared_ptr<Aggregate> SceneParser::parseAccelerator(const nebJson &param) {
    
}

Film * SceneParser::parseFilm(const nebJson &data, Filter * filt) {
    nebJson param = data.GetValue("param", nebJson());
    Film * film = dynamic_cast<Film *>(createFilm(param,{filt}));
    return film;
}

PALADIN_END
