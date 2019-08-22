//
//  film.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/8/20.
//

#include "film.hpp"

PALADIN_BEGIN

Film::Film(const Point2i &resolution, const AABB2f &cropWindow,
           std::unique_ptr<Filter> filter, Float diagonal,
           const std::string &filename, Float scale,
           Float maxSampleLuminance):
fullResolution(resolution),
diagonal(diagonal * .001),
filter(std::move(filter)),
filename(filename),
_scale(scale),
_maxSampleLuminance(maxSampleLuminance) {
    croppedPixelBounds = AABB2i(Point2i(std::ceil(fullResolution.x * cropWindow.pMin.x),
                     std::ceil(fullResolution.y * cropWindow.pMin.y)),
             Point2i(std::ceil(fullResolution.x * cropWindow.pMax.x),
                     std::ceil(fullResolution.y * cropWindow.pMax.y)));
}

PALADIN_END
