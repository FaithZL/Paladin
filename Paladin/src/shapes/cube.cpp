//
//  cube.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/7/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "cube.hpp"

PALADIN_BEGIN

vector<shared_ptr<Shape>> createQuad(shared_ptr<const Transform> o2w,
                                bool reverseOrientation,
                                     int width, int height) {
    if (height == 0) {
        height = width;
    }
    DCHECK(width > 0);
    DCHECK(height > 0);
    Point3f tr(width, height, 0);
    Point3f tl(-width, height, 0);
    Point3f br(width, -height, 0);
    Point3f bl(-width, -height,0);
    
    Point3f points[] = {tl, bl, br, tr};
    int vertIndice[6] = {0,1,2, 0,2,3};
    Point2f UVs[] = {Point2f(0, 1), Point2f(0, 0), Point2f(1, 0), Point2f(1,1)};
    int nTri = 2;
    int nVert = 4;
    auto mesh = createTriMesh(o2w, nTri,vertIndice, nVert, points, UVs);
    vector<shared_ptr<Shape>> ret(nTri);
    shared_ptr<Transform>w2o(o2w->getInverse_ptr());
    for (int i = 0; i < nTri; ++i) {
        ret.push_back(createTri(o2w, w2o, reverseOrientation, mesh, i));
    }
    return ret;
}


PALADIN_END
