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

PALADIN_BEGIN

using namespace neb;
USING_STD;

void SceneParser::parse(const nebJson &data) {
    nebJson filterData = data.GetValue("filter", nebJson());
    _filter = parseFilter(filterData);
    nebJson samplerData = data.GetValue("sampler", nebJson());
    _sampler = parseSampler(samplerData);
    nebJson filmData = data.GetValue("film", nebJson());
    _film = parseFilm(filmData);
}


shared_ptr<Sampler> SceneParser::parseSampler(const nebJson &data) {
    string samplerType = data.GetValue("type", "stratified");
    nebJson param = data.GetValue("param", nebJson());
    auto creator = GET_CREATOR(samplerType);
    shared_ptr<Sampler> ret = dynamic_pointer_cast<Sampler>(creator(param, {}));
    return ret;
}

shared_ptr<Camera> SceneParser::parseCamera(const neb::CJsonObject &param) {
    
}

unique_ptr<Integrator> SceneParser::parseIntegrator(const neb::CJsonObject &filterData) {
    
}

shared_ptr<Filter> SceneParser::parseFilter(const neb::CJsonObject &data) {
    string filterType = data.GetValue("type", "box");
    nebJson param = data.GetValue("param", nebJson());
    auto creator = GET_CREATOR(filterType.c_str());
    shared_ptr<Filter> ret = dynamic_pointer_cast<Filter>(creator(param, {}));
    return ret;
}

shared_ptr<Aggregate> SceneParser::parseAccelerator(const neb::CJsonObject &param) {
    
}

shared_ptr<Film> SceneParser::parseFilm(const neb::CJsonObject &param) {
    
}

PALADIN_END