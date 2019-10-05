//
//  mipmap.h
//  Paladin
//
//  Created by SATAN_Z on 2019/10/4.
//

#ifndef mipmap_h
#define mipmap_h

#include "header.h"

PALADIN_BEGIN

enum class ImageWrap { Repeat, Black, Clamp };
struct ResampleWeight {
    int firstTexel;
    Float weight[4];
};


PALADIN_END

#endif /* mipmap_h */
