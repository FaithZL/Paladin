//
//  light.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "light.hpp"
#include "core/scene.hpp"
#include "math/sampling.hpp"

PALADIN_BEGIN


//bool VisibilityTester::unoccluded(const Scene &scene) const {
//    return !scene.intersectP(_p0.spawnRayTo(_p1));
//}
//
//Spectrum VisibilityTester::tr(const Scene &scene, Sampler &sampler) const {
//    Ray ray(_p0.spawnRayTo(_p1));
//    Spectrum Tr(1.f);
//    while (true) {
//        SurfaceInteraction isect;
//        bool hitSurface = scene.intersect(ray, &isect);
//        // Handle opaque surface along ray's path
//        if (hitSurface && isect.primitive->getMaterial() != nullptr)
//            return Spectrum(0.0f);
//        
//        // Update transmittance for current ray segment
//        if (ray.medium) Tr *= ray.medium->tr(ray, sampler);
//        
//        // Generate next ray segment or return final transmittance
//        if (!hitSurface) break;
//        ray = isect.spawnRayTo(_p1);
//    }
//    return Tr;
//}


PALADIN_END
