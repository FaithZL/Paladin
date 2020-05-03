//
//  triangle.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/5/3.
//

#ifndef triangle_hpp
#define triangle_hpp

#include "core/header.h"
#include "core/shape.hpp"
#include "core/texture.hpp"
#include <vector>
#include "math/frame.hpp"

PALADIN_BEGIN

struct IndexSet {
    IndexSet(int uv, int pos, int normal, int edge):
    uv(uv),
    pos(pos),
    normal(normal),
    edge(edge) {
        
    }
    
    IndexSet(int idx)
    : uv(idx),
    pos(idx),
    normal(idx),
    edge(idx) {
        
    }
    
    IndexSet() {
        uv = 0;
        pos = 0;
        normal = 0;
        edge = 0;
    }
    
    // 顶点uv索引
    int uv;
    // 顶点位置索引
    int pos;
    // 顶点法线索引
    int normal;
    //
    int edge;
};

struct TriangleI {
    
    
    
};

PALADIN_END

#endif /* triangle_hpp */
