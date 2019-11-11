//
//  serializable.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/2.
//

#ifndef serializable_hpp
#define serializable_hpp

#include "core/header.h"

class Serializable {
    
public:
    virtual void fromXML() = 0;
    
};

#endif /* serializable_hpp */
