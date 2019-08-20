//
//  film.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/8/20.
//

#ifndef film_hpp
#define film_hpp

#include "header.h"

PALADIN_BEGIN

struct FilmTilePixel {
    Spectrum contribSum = 0.f;
    Float filterWeightSum = 0.f;
};

PALADIN_END

#endif /* film_hpp */
