//
//  filter.h
//  Paladin
//
//  Created by SATAN_Z on 2019/8/20.
//

#ifndef filter_h
#define filter_h

#include "core/header.h"

PALADIN_BEGIN

class Filter {
    
public:
    virtual ~Filter() {
        
    }
    
    Filter(const Vector2f &radius)
    : radius(radius),
    invRadius(Vector2f(1 / radius.x, 1 / radius.y)) {
        
    }
    
    virtual Float evaluate(const Point2f &p) const = 0;
    
    const Vector2f radius;
    const Vector2f invRadius;
};

PALADIN_END

#endif /* filter_h */
