//
//  sceneparser.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/11.
//

#include "sceneparser.hpp"
#include "tools/classfactory.hpp"
PALADIN_BEGIN

void SceneParser::parse(const neb::CJsonObject &jsonObj) {
    auto p = GET_CREATOR("pathtracer");
    p(jsonObj,{});
}

void SceneParser::parseSampler(const neb::CJsonObject &jsonObj) {
    
}

void SceneParser::parseCamera(const neb::CJsonObject &jsonObj) {
    
}

void SceneParser::parseIntegrator(const neb::CJsonObject &jsonObj) {
    
}

void SceneParser::parseFilter(const neb::CJsonObject &jsonObj) {
    
}

void SceneParser::parseAccelerator(const neb::CJsonObject &jsonObj) {
    
}

void SceneParser::parseFilm(const neb::CJsonObject &jsonObj) {
    
}

PALADIN_END
