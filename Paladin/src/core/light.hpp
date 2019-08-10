//
//  light.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef light_hpp
#define light_hpp

#include "header.h"

PALADIN_BEGIN

class Light {
    
public:
    
    virtual ~Light();
    Light(int flags, const Transform &LightToWorld,
          const MediumInterface &mediumInterface, int nSamples = 1);
};

PALADIN_END

#endif /* light_hpp */
