//
//  fileio.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef fileio_hpp
#define fileio_hpp

#include "core/header.h"

PALADIN_BEGIN

std::unique_ptr<RGBSpectrum[]> readImage(const std::string &name, Point2i *resolution);

void writeImage(const std::string &name, const Float *rgb,
                const AABB2i &outputBounds, const Point2i &totalResolution);

PALADIN_END

#endif /* fileio_hpp */
