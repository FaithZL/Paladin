//
//  transform.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "transform.hpp"

PALADIN_BEGIN

Float Matrix4x4::getDet() const {
    Float result =
    _11 * _22 * _33 * _44   - _11 * _22 * _34 * _43   -
    _11 * _23 * _32 * _44   + _11 * _23 * _34 * _42   +
    
    _11 * _24 * _32 * _43   - _11 * _24 * _33 * _42   -
    _12 * _21 * _33 * _44   + _12 * _21 * _34 * _43   +
    
    _12 * _23 * _31 * _44   - _12 * _23 * _34 * _41   -
    _12 * _24 * _31 * _43   + _12 * _24 * _33 * _41   +
    
    _13 * _21 * _32 * _44   - _13 * _21 * _34 * _42   -
    _13 * _22 * _31 * _44   + _13 * _22 * _34 * _41   +
    
    _13 * _24 * _31 * _42   - _13 * _24 * _32 * _41   -
    _14 * _21 * _32 * _43   + _14 * _21 * _33 * _42   +
    
    _14 * _22 * _31 * _43   - _14 * _22 * _33 * _41   -
    _14 * _23 * _31 * _42   + _14 * _23 * _32 * _41 ; 
    
    return result;
}

Float Matrix4x4::getAdjointElement(Float a1, Float a2, Float a3,
                               Float b1, Float b2, Float b3,
                               Float c1, Float c2, Float c3) const {
    return a1*(b2*c3 - c2*b3) - a2*(b1*c3 - c1*b3) + a3*(b1*c2 - c1*b2);
}

Matrix4x4 Matrix4x4::getAdjointMat() const {
    Float a1 = getAdjointElement(_22, _23, _24, _32, _33, _34, _42, _43, _44);
    Float a2 = getAdjointElement(_21, _23, _24, _31, _33, _34, _41, _43, _44);
    Float a3 = getAdjointElement(_21, _22, _24, _31, _32, _34, _41, _42, _44);
    Float a4 = getAdjointElement(_21, _22, _23, _31, _32, _33, _41, _42, _43);
    Float b1 = getAdjointElement(_12, _13, _14, _32, _33, _34, _42, _43, _44);
    Float b2 = getAdjointElement(_11, _13, _14, _31, _33, _34, _41, _43, _44);
    Float b3 = getAdjointElement(_11, _12, _14, _31, _32, _34, _41, _42, _44);
    Float b4 = getAdjointElement(_11, _12, _13, _31, _32, _33, _41, _42, _43);
    Float c1 = getAdjointElement(_12, _13, _14, _22, _23, _24, _42, _43, _44);
    Float c2 = getAdjointElement(_11, _13, _14, _21, _23, _24, _41, _43, _44);
    Float c3 = getAdjointElement(_11, _12, _14, _21, _22, _24, _41, _42, _44);
    Float c4 = getAdjointElement(_11, _12, _13, _21, _22, _23, _41, _42, _43);
    Float d1 = getAdjointElement(_12, _13, _14, _22, _23, _24, _32, _33, _34);
    Float d2 = getAdjointElement(_11, _13, _14, _21, _23, _24, _31, _33, _34);
    Float d3 = getAdjointElement(_11, _12, _14, _21, _22, _24, _31, _32, _34);
    Float d4 = getAdjointElement(_11, _12, _13, _21, _22, _23, _31, _32, _33);
    
    Float arr[16] = {
        a1, -a2, a3, -a4,
        -b1, b2, -b3, b4,
        c1, -c2, c3, -c4,
        -d1, d2, -d3, d4
    };
    return Matrix4x4(arr).getTransposeMat();
}

// 伴随矩阵除以行列式
Matrix4x4 Matrix4x4::getInverseMat() const {
    Matrix4x4 adj = getAdjointMat();
    Float det = getDet();
    return adj / (det);
}

Matrix4x4 Matrix4x4::getTransposeMat() const {
    Float a[16] = {
        _11, _21, _31, _41,
        _12, _22, _32, _42,
        _13, _23, _33, _43,
        _14, _24, _34, _44
    };
    return Matrix4x4(a);
}

Matrix4x4 Matrix4x4::identity() {
    Float ret[16] = {
        1 , 0 , 0 , 0,
        0 , 1 , 0 , 0,
        0 , 0 , 1 , 0,
        0 , 0 , 0 , 1
    };
    return Matrix4x4(ret);
}

Matrix4x4 Matrix4x4::operator * (const Matrix4x4 &other) const {
    Float ret[4][4];
    for (int i = 0 ; i < 4 ; ++ i) {
        for (int j = 0 ; j < 4 ; ++ j) {
            ret[i][j] = m[i][0] * other.m[0][j]
                    + m[i][1] * other.m[1][j]
                    + m[i][2] * other.m[2][j]
                    + m[i][3] * other.m[3][j];
        }
    }
    return Matrix4x4(ret);
}

Matrix4x4 Matrix4x4::operator * (Float num) const {
    Float t[16] = {};
    for (int i = 0 ; i < 16 ; ++ i) {
        t[i] = a[i] * num;
    }
    return Matrix4x4(t);
}

Matrix4x4 Matrix4x4::operator / (Float num) const {
    Float t[16] = {};
    for (int i = 0 ; i < 16 ; ++ i) {
        t[i] = a[i] / num;
    }
    return Matrix4x4(t);
}

Matrix4x4 Matrix4x4::operator - (const Matrix4x4 &other) const {
    Float ret[16];
    for (int i = 0 ; i < 16; ++ i) {
        ret[i] = a[i] - other.a[i];
    }
    return Matrix4x4(ret);
}

Matrix4x4 Matrix4x4::operator + (const Matrix4x4 &other) const {
    Float ret[16];
    for (int i = 0 ; i < 16; ++ i) {
        ret[i] = a[i] + other.a[i];
    }
    return Matrix4x4(ret);
}

bool Matrix4x4::operator == (const Matrix4x4 &other) const {
    for (int i = 0 ; i < 16 ; ++ i) {
        if (a[i] != other.a[i]) {
            return false;
        }
    }
    return true;
}

bool Matrix4x4::operator < (const Matrix4x4 &other) const {
    for (int i = 0; i < 16; ++ i) {
        if (a[i] >= other.a[i]) {
            return false;
        }
    }
    return true;
}

bool Matrix4x4::operator != (const Matrix4x4 &other) const {
    for (int i = 0; i < 16; ++ i) {
        if (a[i] == other.a[i]) {
            return false;
        }
    }
    return true;
}

PALADIN_END
