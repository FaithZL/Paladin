//
//  constant.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef constant_h
#define constant_h

#include "core/texture.hpp"
#include "core/header.h"

PALADIN_BEGIN

template <typename T>
class ConstantTexture : public Texture<T> {
public:

    ConstantTexture(const T &value) : _value(value) {
        
    }
    
    virtual T evaluate(const SurfaceInteraction &) const {
        return _value;
    }
    
    virtual nloJson toJson() const override {
        return nloJson();
    }
    
private:
    T _value;
};

PALADIN_END

#endif /* constant_hpp */
