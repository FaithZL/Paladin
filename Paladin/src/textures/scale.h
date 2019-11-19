//
//  scale.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/1.
//

#ifndef scale_h
#define scale_h

#include "core/header.h"
#include "core/texture.hpp"

PALADIN_BEGIN

template <typename T1, typename T2>
class ScaleTexture : public Texture<T2> {
public:
    ScaleTexture(const std::shared_ptr<Texture<T1>> &tex1,
                 const std::shared_ptr<Texture<T2>> &tex2)
    : _tex1(tex1), _tex2(tex2) {
        
    }
    
    virtual T2 evaluate(const SurfaceInteraction &si) const {
        return _tex1->evaluate(si) * _tex2->evaluate(si);
    }
    
    virtual nloJson toJson() const override {
        return nloJson();
    }
    
private:
    std::shared_ptr<Texture<T1>> _tex1;
    std::shared_ptr<Texture<T2>> _tex2;
};


PALADIN_END


#endif /* scale_h */
