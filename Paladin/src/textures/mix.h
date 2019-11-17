//
//  mix.h
//  Paladin
//
//  Created by SATAN_Z on 2019/10/3.
//

#ifndef texture_mix_h
#define texture_mix_h

#include "core/header.h"
#include "core/texture.hpp"

PALADIN_BEGIN

template <typename T>
class MixTexture : public Texture<T> {
public:
    MixTexture(const std::shared_ptr<Texture<T>> &tex1,
               const std::shared_ptr<Texture<T>> &tex2,
               const std::shared_ptr<Texture<Float>> &amount)
    : _tex1(tex1), _tex2(tex2), _amount(amount) {
        
    }
    
    virtual T evaluate(const SurfaceInteraction &si) const {
        T t1 = _tex1->evaluate(si), t2 = _tex2->evaluate(si);
        Float amt = _amount->evaluate(si);
        return (1 - amt) * t1 + amt * t2;
    }
    
    virtual nebJson toJson() const override {
        return nebJson();
    }
    
private:
    std::shared_ptr<Texture<T>> _tex1, _tex2;
    std::shared_ptr<Texture<Float>> _amount;
};

PALADIN_END

#endif /* texture_mix_h */
