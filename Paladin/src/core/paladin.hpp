//
//  paladin.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef paladin_hpp
#define paladin_hpp

#include "parser/sceneparser.hpp"
#include "tools/classfactory.hpp"
#include "integrators/pathtracer.hpp"
PALADIN_BEGIN

class Paladin {
public:
    Paladin() {
        
    }
    
    int run() {
        return 0;
    }
    
    void render(const std::string fileName) {
        auto p = ClassFactory::getInstance()->getCreatorByName("pathtracer");
//        auto f = (createObject)p;
        neb::CJsonObject j;
        
        p(j);
//        _sceneParser.loadFromJson(fileName);
    }
    
private:
    
    SceneParser _sceneParser;

};

PALADIN_END

#endif /* paladin_hpp */
