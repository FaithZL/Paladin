//
//  quad.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/24.
//

#include "quad.hpp"

PALADIN_BEGIN

Quad::Quad(shared_ptr<const Transform> o2w,
            shared_ptr<const Transform> w2o,
            bool reverseOrientation,
            int width, int height)
: Shape(o2w, w2o, reverseOrientation){
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
    Point2f UVs[] = {Point2f()};
    
    auto m = new TriangleMesh(o2w, 2,vertIndice, 4, points, nullptr, nullptr, nullptr);
    
    _mesh.reset(m);
}


PALADIN_END
