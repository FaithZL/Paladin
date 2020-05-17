//
//  instance.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/5/17.
//

#ifndef instance_hpp
#define instance_hpp

#include "core/header.h"
#include "core/shape.hpp"

PALADIN_BEGIN

class Instance : public Shape {
    
public:
    Instance(const Transform * ObjectToWorld, const Transform *WorldToObject,
             const MediumInterface &mi, const Shape * shape,
             const shared_ptr<const Material> &mat = nullptr)
    : Shape(mult_ptr(ObjectToWorld, shape->worldToObject),
            mult_ptr(ObjectToWorld, shape->worldToObject)->getInverse_ptr(),
            shape->reverseOrientation),
    _shape(shape) {
        
    }
    
    virtual bool rayIntersect(const Ray &ray,
                            SurfaceInteraction * isect,
                            bool testAlphaTexture = true) const override;
    
    virtual bool rayOccluded(const Ray &ray, bool testAlphaTexture = true) const override;
    
    void initial();
    
private:
    const Shape * _shape;
    
};

PALADIN_END

#endif /* instance_hpp */
