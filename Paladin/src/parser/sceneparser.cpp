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

void SceneParser::parse(const neb::CJsonObject &data) {
    CJsonObject filterData;
    filterData = data.getValue("filter", filterData);
    _filter = parseFilter(filterData);
}


shared_ptr<Sampler> SceneParser::parseSampler(const neb::CJsonObject &param) {
    
}

shared_ptr<Camera> SceneParser::parseCamera(const neb::CJsonObject &param) {
    
}

shared_ptr<Integrator> SceneParser::parseIntegrator(const neb::CJsonObject &filterData) {
    
}

shared_ptr<Filter> SceneParser::parseFilter(const neb::CJsonObject &data) {
    string filterType = data.getValue("type", "box");
    neb::CJsonObject param;
    param = data.getValue("param", param);
    auto creator = GET_CREATOR(filterType.c_str());
    shared_ptr<Serializable> ret = creator(param, {});
    return nullptr;
}

shared_ptr<Aggregate> SceneParser::parseAccelerator(const neb::CJsonObject &param) {
    
}

shared_ptr<Film> SceneParser::parseFilm(const neb::CJsonObject &param) {
    
}

PALADIN_END
