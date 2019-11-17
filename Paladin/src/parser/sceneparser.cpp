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
#include "cameras/perspective.hpp"

PALADIN_BEGIN

using namespace neb;
USING_STD;

void SceneParser::parse(const nebJson &data) {
    nebJson filterData = data.GetValue("filter", nebJson());
    auto filter = parseFilter(filterData);
    nebJson samplerData = data.GetValue("sampler", nebJson());
    auto sampler = parseSampler(samplerData);
    nebJson filmData = data.GetValue("film", nebJson());
    _film = parseFilm(filmData, filter);
    nebJson cameraData = data.GetValue("camera", nebJson());
    _camera = parseCamera(cameraData);
}


Sampler * SceneParser::parseSampler(const nebJson &data) {
    string samplerType = data.GetValue("type", "stratified");
    nebJson param = data.GetValue("param", nebJson());
    auto creator = GET_CREATOR(samplerType);
    auto ret = dynamic_cast<Sampler *>(creator(param, {}));
    return ret;
}

shared_ptr<const Camera> SceneParser::parseCamera(const nebJson &data) {
    string cameraType = data.GetValue("type", "perspective");
    nebJson param = data.GetValue("param", nebJson());
    auto creator = GET_CREATOR(cameraType);
    auto ret = dynamic_cast<Camera *>(creator(param, {}));
    return shared_ptr<const Camera>(ret);
}

unique_ptr<Integrator> SceneParser::parseIntegrator(const neb::CJsonObject &filterData) {
    
}

Filter * SceneParser::parseFilter(const nebJson &data) {
    string filterType = data.GetValue("type", "box");
    nebJson param = data.GetValue("param", nebJson());
    auto creator = GET_CREATOR(filterType.c_str());
    auto ret = dynamic_cast<Filter *>(creator(param, {}));
    DCHECK(ret != nullptr);
    return ret;
}

shared_ptr<Aggregate> SceneParser::parseAccelerator(const neb::CJsonObject &param) {
    
}

shared_ptr<Film> SceneParser::parseFilm(const nebJson &data, Filter * filt) {
    nebJson param = data.GetValue("param", nebJson());
    Film * film = dynamic_cast<Film *>(createFilm(param,{filt}));
    return shared_ptr<Film>(film);
}

PALADIN_END
