//
//  cone.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "cone.hpp"

PALADIN_BEGIN

bool Cone::intersect(const paladin::Ray &ray, Float *tHit, paladin::SurfaceInteraction *isect, bool testAlphaTexture) const {
    // todo
    return true;
}
// todo
bool Cone::intersectP(const paladin::Ray &ray, bool testAlphaTexture) const {
    return true;
}
// todo
Interaction Cone::sampleA(const Point2f &u, Float *pdf) const {
    return Interaction();
}

PALADIN_END
