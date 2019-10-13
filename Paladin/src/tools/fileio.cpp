//
//  fileio.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "fileio.hpp"
#include "tools/fileutil.hpp"
#include "core/spectrum.hpp"
#include "stb_image.h"

PALADIN_BEGIN

static RGBSpectrum *ReadImagePNG(const std::string &name, int *width,
                                 int *height) {
    // todo
}

std::unique_ptr<RGBSpectrum[]> readImage(const std::string &name, Point2i *resolution) {
    if (hasExtension(name, "png")) {
        return std::unique_ptr<RGBSpectrum []>(ReadImagePNG(name, &resolution->x, &resolution->y));
    }
}

PALADIN_END
