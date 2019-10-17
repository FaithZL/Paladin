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


bool VisibilityTester::unoccluded(const Scene &scene) const {
    return !scene.intersectP(_p0.spawnRayTo(_p1));
}



PALADIN_END
