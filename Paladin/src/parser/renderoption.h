//
//  renderoption.h
//  Paladin
//
//  Created by SATAN_Z on 2019/11/11.
//

#ifndef renderoption_h
#define renderoption_h

#include "core/header.h"
#include "math/transform.hpp"

PALADIN_BEGIN

USING_STD

struct RenderOption {
    
    Float transformStartTime = 0;
    
    Float transformEndTime = 0;
    
    string samplerName = "stratified";
    
    string integratorName = "pathtracer";
    
    string filterName = "box";
    
    string acceleratorName = "bvh";
    
    string filmName = "image.png";
    
    string cameraName = "perspective";
    
    vector<shared_ptr<Light>> lights;
    
    vector<shared_ptr<Primitive>> primitives;
    
    shared_ptr<Transform *> cameraToWorld;
};

PALADIN_END

#endif /* renderoption_h */
