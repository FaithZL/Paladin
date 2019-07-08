//
//  transform.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "transform.hpp"

PALADIN_BEGIN

Matrix4x4::Matrix4x4(Float mm[4][4]) {
    memcpy(m, mm, 16 * sizeof(Float));
    assert(!hasNaNs());
}

Matrix4x4 Transpose(const Matrix4x4 &mm) {
    assert(!mm.hasNaNs());
    return Matrix4x4(mm.m[0][0], mm.m[1][0], mm.m[2][0], mm.m[3][0], mm.m[0][1],
                     mm.m[1][1], mm.m[2][1], mm.m[3][1], mm.m[0][2], mm.m[1][2],
                     mm.m[2][2], mm.m[3][2], mm.m[0][3], mm.m[1][3], mm.m[2][3],
                     mm.m[3][3]);
}


PALADIN_END
