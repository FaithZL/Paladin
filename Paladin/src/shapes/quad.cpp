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
: Shape(o2w, w2o, reverseOrientation) {
    Point3f tr(width, height, 0);
    Point3f tl(-width, height, 0);
    Point3f br(width, -height, 0);
    Point3f bl(-width, -height,0);
    
    Point3f p[] = {tr,br,bl, tl};
    int vi[6] = {0,1,2, 0,2,3};
    
    auto m = new TriangleMesh(o2w, 2, vi, 4, p, nullptr, nullptr, nullptr,nullptr,nullptr,nullptr);
    
    _mesh.reset(m);
}


PALADIN_END
