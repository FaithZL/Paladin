//
//  texture.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef texture_hpp
#define texture_hpp

#include "header.h"
#include "interaction.hpp"

PALADIN_BEGIN

template <typename T>
class Texture {
public:
    virtual T evaluate(const SurfaceInteraction &) const = 0;
    
    virtual ~Texture() {
        
    }
};

PALADIN_END

#endif /* texture_hpp */
