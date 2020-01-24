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
#include "tools/parallel.hpp"

PALADIN_BEGIN

class Paladin {
public:
    
    int run() {
        return 0;
    }
    
    void render(const std::string &fileName) {
        _sceneParser.loadFromJson(fileName);
        parallelCleanup();
    }
    
    static Paladin * getInstance();
    
    const SceneParser * getSceneParser() const {
        return & _sceneParser;
    }
    
private:
    
    Paladin() {
        
    }
    
    static Paladin * s_paladin;
    
    SceneParser _sceneParser;

};

PALADIN_END

#endif /* paladin_hpp */
