//
//  bvh.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef bvh_hpp
#define bvh_hpp

#include "header.h"
#include "primitive.hpp"
PALADIN_BEGIN

class BVHAccel : public Aggregate {
    
    enum class SplitMethod { SAH, HLBVH, Middle, EqualCounts };
    
};

PALADIN_END

#endif /* bvh_hpp */
