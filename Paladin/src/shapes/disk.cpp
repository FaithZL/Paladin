//
//  disk.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "disk.hpp"

PALADIN_BEGIN

bool Disk::intersectP(const paladin::Ray &ray, bool testAlphaTexture) const {
    // todo
    return false;
}

bool Disk::intersect(const paladin::Ray &ray, Float *tHit, paladin::SurfaceInteraction *isect, bool testAlphaTexture) const {
    // todo
    return false;
}

Interaction Disk::sampleA(const Point2f &u, Float *pdf) const {
    Interaction ret;
    // todo 
    return ret;
}

PALADIN_END
