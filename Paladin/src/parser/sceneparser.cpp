//
//  sceneparser.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/11.
//

#include "sceneparser.hpp"
#include "tools/classfactory.hpp"
#include "core/film.hpp"

PALADIN_BEGIN

using namespace neb;
USING_STD;

void SceneParser::parse(const neb::CJsonObject &param) {
    CJsonObject filmParam;
    filmParam = param.getValue("film", filmParam);
    Film * film = parseFilm(filmParam);
    CJsonObject cameraParam;
    cameraParam = param.getValue("camera", cameraParam);
    
}

Sampler * SceneParser::parseSampler(const neb::CJsonObject &param) {
    
}

Camera * SceneParser::parseCamera(const neb::CJsonObject &param) {
    
}

Integrator * SceneParser::parseIntegrator(const neb::CJsonObject &param) {
    
}

Filter * SceneParser::parseFilter(const neb::CJsonObject &param) {
    
}

Aggregate * SceneParser::parseAccelerator(const neb::CJsonObject &param) {
    
}

Film * SceneParser::parseFilm(const neb::CJsonObject &param) {
    
}

PALADIN_END
