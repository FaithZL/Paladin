//
//  quaternion.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef quaternion_hpp
#define quaternion_hpp

#include "header.h"

PALADIN_BEGIN

struct Quaternion {
    
    Quaternion() : _v(0, 0, 0), _w(1) {}
    
    Vector3f _v;
    Float _w;
};

PALADIN_END

#endif /* quaternion_hpp */
