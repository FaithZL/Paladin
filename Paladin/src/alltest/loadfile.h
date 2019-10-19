//
//  loadfile.h
//  Paladin
//
//  Created by SATAN_Z on 2019/10/18.
//

#ifndef loadfile_h
#define loadfile_h
#include "tools/fileio.hpp"
#include "core/spectrum.hpp"

PALADIN_BEGIN

void loadfile() {
    auto pt = "res/derelict_overpass_1k.hdr";
    Point2i res;
    
    std::unique_ptr<RGBSpectrum[]> ret = readImage(pt, &res);
    AABB2i crop(Point2i(0,0), res);
    std::unique_ptr<Float[]> cp(new Float[res.x * res.y * 3]);
    for (int i = 0; i < res.x * res.y; ++i) {
        ret[i].ToRGB(&cp[i * 3]);
    }
    auto p2 = "res/test_hdr999.hdr";
    writeImage(p2, &cp[0],crop , res);
}

PALADIN_END

#endif /* loadfile_h */
