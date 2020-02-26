//
//  endpoint.h
//  Paladin
//
//  Created by SATAN_Z on 2020/2/4.
//

#ifndef endpoint_h
#define endpoint_h

#include "core/interaction.hpp"

PALADIN_BEGIN

struct EndpointInteraction : public Interaction {
    union {
        const Camera *camera;
        const Light *light;
    };
    
    EndpointInteraction() : Interaction(), light(nullptr) {
        
    }
    
    EndpointInteraction(const Interaction &it, const Camera *camera)
    : Interaction(it),
    camera(camera) {
        
    }
    
    EndpointInteraction(const Camera *camera, const Ray &ray)
    : Interaction(ray.ori, ray.time, ray.medium), camera(camera) {
        
    }
    
    EndpointInteraction(const Light *light, const Ray &r, const Normal3f &nl)
    : Interaction(r.ori, r.time, r.medium), light(light) {
        normal = nl;
    }
    
    EndpointInteraction(const Interaction &it, const Light *light)
    : Interaction(it), light(light) {
        
    }
    
    EndpointInteraction(const Ray &ray)
    : Interaction(ray.at(1), ray.time, ray.medium), light(nullptr) {
        normal = Normal3f(-ray.dir);
    }
    
};


PALADIN_END

#endif /* endpoint_h */
