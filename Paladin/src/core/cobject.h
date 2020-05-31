//
//  cobject.h
//  Paladin
//
//  Created by SATAN_Z on 2019/11/2.
//

#ifndef CObject_h
#define CObject_h

#include "core/header.h"

PALADIN_BEGIN

class CObject {
    
public:
    virtual nloJson toJson() const {
        DCHECK(false);
        return nloJson();
    }
    
    virtual ~CObject() {
        
    }
};

PALADIN_END

#endif /* CObject_h */
