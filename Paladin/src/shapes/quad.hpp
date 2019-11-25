//
//  quad.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/24.
//

#ifndef quad_hpp
#define quad_hpp

#include "shapes/trianglemesh.hpp"

PALADIN_BEGIN
//
class Quad : public Shape {
    
public:
    Quad(shared_ptr<const Transform> o2w,
         shared_ptr<const Transform> w2o,
         bool reverseOrientation,
         int width, int height = 0);
    
    virtual nloJson toJson() const override {
        return nloJson();
    }
    
private:
    
    unique_ptr<TriangleMesh> _mesh;
};

PALADIN_END

#endif /* quad_hpp */
