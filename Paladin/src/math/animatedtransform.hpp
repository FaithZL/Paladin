//
//  animatedtransform.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/7/13.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef animatedtransform_hpp
#define animatedtransform_hpp

#include "transform.hpp"

PALADIN_BEGIN

class AnimatedTransform {
    
public:
    
    const Transform * _startTransform;
    const Transform * _endTransform;
    const Float _startTime;
    const Float _endTime;
    const bool _actuallyAnimated;
    
    
};

PALADIN_END

#endif /* animatedtransform_hpp */
