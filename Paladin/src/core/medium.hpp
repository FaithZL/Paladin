//
//  medium.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/7/16.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef medium_hpp
#define medium_hpp

#include "header.h"

PALADIN_BEGIN

// 两个介质的相交处，nullptr表示真空
struct MediumInterface {
    
    MediumInterface(const Medium *medium) : inside(medium), outside(medium) {
        
    }
    
    MediumInterface(const Medium *inside = nullptr, const Medium *outside = nullptr)
    :inside(inside),
    outside(outside) {
        
    }
    
    bool IsMediumTransition() const {
        return inside != outside;
    }
    
    const Medium *inside, *outside;
};

PALADIN_END

#endif /* medium_hpp */
