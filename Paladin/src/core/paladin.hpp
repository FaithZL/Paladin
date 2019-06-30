//
//  paladin.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef paladin_hpp
#define paladin_hpp

#include <iostream>
#include "header.hpp"



class Scene;
class Integrator;

PALADIN_BEGIN
    
class Paladin {
    
public:
    
    int run(int argc, const char * argv[]);
    
    Scene * m_scene;
    
    Integrator * m_integrator;
};

PALADIN_END

#endif /* paladin_hpp */
