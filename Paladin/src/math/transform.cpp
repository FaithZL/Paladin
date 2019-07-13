//
//  transform.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "transform.hpp"

PALADIN_BEGIN

//Matrix4x4
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
            ret[i][j] = _m[i][0] * other._m[0][j]
                    + _m[i][1] * other._m[1][j]
                    + _m[i][2] * other._m[2][j]
                    + _m[i][3] * other._m[3][j];
        }
    }
    return Matrix4x4(ret);
}

Matrix4x4 Matrix4x4::operator * (Float num) const {
    Float t[16] = {};
    for (int i = 0 ; i < 16 ; ++ i) {
        t[i] = _a[i] * num;
    }
    return Matrix4x4(t);
}

Matrix4x4 Matrix4x4::operator / (Float num) const {
    Float t[16] = {};
    for (int i = 0 ; i < 16 ; ++ i) {
        t[i] = _a[i] / num;
    }
    return Matrix4x4(t);
}

Matrix4x4 Matrix4x4::operator - (const Matrix4x4 &other) const {
    Float ret[16];
    for (int i = 0 ; i < 16; ++ i) {
        ret[i] = _a[i] - other._a[i];
    }
    return Matrix4x4(ret);
}

Matrix4x4 Matrix4x4::operator + (const Matrix4x4 &other) const {
    Float ret[16];
    for (int i = 0 ; i < 16; ++ i) {
        ret[i] = _a[i] + other._a[i];
    }
    return Matrix4x4(ret);
}

bool Matrix4x4::operator == (const Matrix4x4 &other) const {
    for (int i = 0 ; i < 16 ; ++ i) {
        if (_a[i] != other._a[i]) {
            return false;
        }
    }
    return true;
}

bool Matrix4x4::operator < (const Matrix4x4 &other) const {
    for (int i = 0; i < 16; ++ i) {
        if (_a[i] >= other._a[i]) {
            return false;
        }
    }
    return true;
}

bool Matrix4x4::operator != (const Matrix4x4 &other) const {
    for (int i = 0; i < 16; ++ i) {
        if (_a[i] == other._a[i]) {
            return false;
        }
    }
    return true;
}

bool Matrix4x4::isIdentity() const {
    return (_m[0][0] == 1.f && _m[0][1] == 0.f && _m[0][2] == 0.f &&
            _m[0][3] == 0.f && _m[1][0] == 0.f && _m[1][1] == 1.f &&
            _m[1][2] == 0.f && _m[1][3] == 0.f && _m[2][0] == 0.f &&
            _m[2][1] == 0.f && _m[2][2] == 1.f && _m[2][3] == 0.f &&
            _m[3][0] == 0.f && _m[3][1] == 0.f && _m[3][2] == 0.f &&
            _m[3][3] == 1.f);
}


//Transform
Transform Transform::operator * (const Transform &other) const {
    return Transform(_mat * other._mat, other._matInv * _matInv);
}

Transform Transform::translate(const Vector3f &delta) {
    Float a[16] = {
        1, 0, 0, delta.x,
        0, 1, 0, delta.y,
        0, 0, 1, delta.z,
        0, 0, 0, 1,
    };
    Float inv[16] = {
        1, 0, 0, -delta.x,
        0, 1, 0, -delta.y,
        0, 0, 1, -delta.z,
        0, 0, 0, 1,
    };
    Matrix4x4 mat(a);
    Matrix4x4 matInv(inv);
    return Transform(mat, matInv);
}

Transform Transform::scale(Float x, Float y, Float z) {
        Float a[16] = {
            x, 0, 0, 0,
            0, y, 0, 0,
            0, 0, z, 0,
            0, 0, 0, 1,
        };
        Float inv[16] = {
            1/x, 0,   0,   0,
            0,   1/y, 0,   0,
            0,   0,   1/z, 0,
            0,   0,   0,   1,
        };
        Matrix4x4 mat(a);
        Matrix4x4 matInv(inv);
        return Transform(mat, matInv);
    }

Transform Transform::scale(Float s) {
    return Transform::scale(s, s, s);
}

Transform Transform::rotateX(Float theta, bool bRadian/*=false*/) {
    theta = bRadian ? theta : degree2radian(theta);
    Float sinTheta = std::sin(theta);
    Float cosTheta = std::cos(theta);
    Float a[16] = {
        1, 0,        0,         0,
        0, cosTheta, -sinTheta, 0,
        0, sinTheta, cosTheta,  0,
        0, 0,        0,         1
    };
    Matrix4x4 mat(a);
    // 旋转矩阵的逆矩阵为该矩阵的转置矩阵
    return Transform(mat, mat.getTransposeMat());
}

Transform Transform::rotateY(Float theta, bool bRadian/*=false*/) {
    theta = bRadian ? theta : degree2radian(theta);
    Float sinTheta = std::sin(theta);
    Float cosTheta = std::cos(theta);
    Float a[16] = {
        cosTheta,  0, sinTheta, 0,
        0,         1, 0,        0,
        -sinTheta, 0, cosTheta, 0,
        0,         0, 0,        1
    };
    Matrix4x4 mat(a);
    // 旋转矩阵的逆矩阵为该矩阵的转置矩阵
    return Transform(mat, mat.getTransposeMat());
}

Transform Transform::rotateZ(Float theta, bool bRadian/*=false*/) {
    theta = bRadian ? theta : degree2radian(theta);
    Float sinTheta = std::sin(theta);
    Float cosTheta = std::cos(theta);
    Float a[16] = {
        cosTheta, -sinTheta, 0, 0,
        sinTheta,  cosTheta, 0, 0,
        0,         0,        1, 0,
        0,         0,        0, 1
    };
    Matrix4x4 mat(a);
    // 旋转矩阵的逆矩阵为该矩阵的转置矩阵
    return Transform(mat, mat.getTransposeMat());
}

Transform Transform::rotate(Float theta, const Vector3f &axis, bool bRadian/*=false*/) {
    Vector3f a = paladin::normalize(axis);
    theta = bRadian ? theta : degree2radian(theta);
    Float sinTheta = std::sin(theta);
    Float cosTheta = std::cos(theta);
    Matrix4x4 mat;

    mat._m[0][0] = a.x * a.x + (1 - a.x * a.x) * cosTheta;
    mat._m[0][1] = a.x * a.y * (1 - cosTheta) - a.z * sinTheta;
    mat._m[0][2] = a.x * a.z * (1 - cosTheta) + a.y * sinTheta;
    mat._m[0][3] = 0;

    mat._m[1][0] = a.x * a.y * (1 - cosTheta) + a.z * sinTheta;
    mat._m[1][1] = a.y * a.y + (1 - a.y * a.y) * cosTheta;
    mat._m[1][2] = a.y * a.z * (1 - cosTheta) - a.x * sinTheta;
    mat._m[1][3] = 0;

    mat._m[2][0] = a.x * a.z * (1 - cosTheta) - a.y * sinTheta;
    mat._m[2][1] = a.y * a.z * (1 - cosTheta) + a.x * sinTheta;
    mat._m[2][2] = a.z * a.z + (1 - a.z * a.z) * cosTheta;
    mat._m[2][3] = 0;
    // 旋转矩阵的逆矩阵为该矩阵的转置矩阵
    return Transform(mat, mat.getTransposeMat());
}

Transform Transform::lookAt(const Point3f &pos, const Point3f &look, const Vector3f &up) {
    //基本思路，先用up向量与dir向量确定right向量
    // right向量与dir向量互相垂直，由此可以确定新的up向量
    // right，dir，newUp向量两两垂直，可以构成直角坐标系，也就是视图空间
    Vector3f dir = normalize(look - pos);
    Vector3f right = cross(normalize(up), dir);
    if (right.lengthSquared() == 0) {
        // dir与up向量共线不合法
        return Transform();
    }
    right = normalize(right);
    Vector3f newUp = cross(dir, right);
    Float a[16] = {
        right.x, newUp.x, dir.x, pos.x,
        right.y, newUp.y, dir.y, pos.y,
        right.z, newUp.z, dir.z, pos.z,
        0,       0,       0,     1
    };
    Matrix4x4 cameraToWorld(a);
    return Transform(cameraToWorld.getInverseMat(), cameraToWorld);
}

Transform Transform::orthographic(Float zNear, Float zFar) {
    Float a[16] = {
        1, 0, 0,                  0,
        0, 1, 0,                  0,
        0, 0, 1 / (zFar - zNear), -zNear,
        0, 0, 0,                  1,
    };
    return Transform(Matrix4x4(a));
}

Transform Transform::perspective(Float fov, Float zNear, Float zFar, bool bRadian/*=false*/) {
    //这里的透视矩阵没有aspect参数，暂时不知道具体原因，等待后续了解
    fov = bRadian ? fov : degree2radian(fov);
    Float invTanAng = 1 / std::tan(fov / 2);
    Float a[16] = {
        invTanAng, 0, 0,             0,
        0, invTanAng, 0,             0,
        0, 0, zFar / (zFar - zNear), -zFar * zNear / (zFar - zNear),
        0, 0,         1,             0
    };
    Matrix4x4 mat(a);
    return Transform(mat);
}

PALADIN_END












