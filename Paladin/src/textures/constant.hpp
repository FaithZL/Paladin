//
//  constant.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/21.
//

#ifndef constant_hpp
#define constant_hpp

#include "core/texture.hpp"
#include "core/header.h"

PALADIN_BEGIN

template <typename T>
class ConstantTexture : public Texture<T> {
public:

    ConstantTexture(const T &value) : _value(value) {
        
    }
    
    virtual T evaluate(const SurfaceInteraction &) const override {
        return _value;
    }
    
    virtual nloJson toJson() const override {
        return nloJson();
    }
    
    static shared_ptr<ConstantTexture<Spectrum>> create(const Float rgb[3]) {
        return make_shared<ConstantTexture<Spectrum>>(Spectrum::FromRGB(rgb));
    }
    
    static shared_ptr<ConstantTexture<T>> create(T v) {
        return make_shared<ConstantTexture<T>>(v);
    }
    
private:
    T _value;
};

CObject_ptr createFloatConstant(const nloJson &param, const Arguments &lst);

CObject_ptr createSpectrumConstant(const nloJson &param, const Arguments &lst);

PALADIN_END

#endif /* constant_hpp */
