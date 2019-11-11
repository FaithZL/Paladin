//
//  serializable.h
//  Paladin
//
//  Created by SATAN_Z on 2019/11/2.
//

#ifndef serializable_h
#define serializable_h

#include "core/header.h"

PALADIN_BEGIN

class Serializable {
    
public:
    virtual neb::CJsonObject toJson() {
        return neb::CJsonObject();
    }
};

PALADIN_END

#endif /* serializable_h */
