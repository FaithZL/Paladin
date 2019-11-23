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

/*
 个人对于以下函数的理解
 在pbrt中，对于SurfaceInteraction的变换，应该都是不同坐标系之间的坐标转换
 而不是物体自身的平移旋转缩放的变换
 因为如果SurfaceInteraction发生了方向或位置变化，那么du/dx这几个为标量的偏导数
 必定会发生变化

 延伸出以下总结
 各个坐标系之前的转换通常仅仅是平移跟旋转的组合
 所以经过变换后的向量，模长是不变的
*/
SurfaceInteraction Transform::exec(const paladin::SurfaceInteraction &isect) const {
    SurfaceInteraction ret;
    
    // 基类Interaction中的成员
    ret.pos = exec(isect.pos, isect.pError, &ret.pError);
    ret.normal = normalize(exec(isect.normal));
    ret.time = isect.time;
    ret.wo = normalize(exec(isect.wo));
    ret.mediumInterface = isect.mediumInterface;
    
    // 不需要变换的对象
    ret.bsdf = isect.bsdf;
    ret.bssrdf = isect.bssrdf;
    ret.primitive = isect.primitive;
    ret.shape = isect.shape;
    ret.faceIndex = isect.faceIndex;
    ret.uv = isect.uv;
    
    // 法向量对uv参数的导数
    ret.dndu = exec(isect.dndu);
    ret.dndv = exec(isect.dndv);
    // 表面上点对uv参数的导数
    ret.dpdu = exec(isect.dpdu);
    ret.dpdv = exec(isect.dpdv);
    // 表面上的点对屏幕坐标的导数
    ret.dpdx = exec(isect.dpdx);
    ret.dpdy = exec(isect.dpdy);

    // todo 这部分为何没有变换，有点不明白
    // 个人理解，对于SurfaceInteraction的变换只能是不同坐标系之间的坐标转换
    // 而不是物体的平移旋转缩放，所以以下偏导数不会变化
    ret.dudx = isect.dudx;
    ret.dudy = isect.dudy;
    ret.dvdx = isect.dvdx;
    ret.dvdy = isect.dvdy;

    // 着色参数变换
    ret.shading.normal = normalize(exec(isect.shading.normal));
    ret.shading.dpdu = exec(isect.shading.dpdu);
    ret.shading.dpdv = exec(isect.shading.dpdv);
    ret.shading.dndu = exec(isect.shading.dndu);
    ret.shading.dndv = exec(isect.shading.dndv);
    // 使着色法线和结构法线在表面同侧
    ret.shading.normal = faceforward(ret.shading.normal, ret.normal);
    
    return ret;
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
        DCHECK(false);
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

/*
要把相机空间xy分辨映射到[-1,1]中，z映射到[0,1]中
x'只与x以及z有关
y'只与y以及z有关
因为z'与xy均无关
可以假设投影矩阵如下
X, 0, 0, 0, 
0, Y, 0, 0,
0, 0, A, B, 
0, 0, 1, 0,

从相机空间映射到视平面上
透视效果，近大远小，可得
x' = x/z
y' = y/z
将 z范围映射到[0,1]之间，可得
经过
z' = (Az + B)/z
当 z = f时，z' = 1
当 z = n时，z' = 0
A = f/(f - n)
B = -fn/(f - n)
带入得
z' = f(z-n) / z(f-n)
由此可推导出透视投影矩阵如下

这里的推导过程不清晰，以后补全todo
*/
Transform Transform::perspective(Float fov, Float zNear, Float zFar, bool bRadian/*=false*/) {
    //这里的透视矩阵没有aspect参数，是因为把光栅空间的变换分离出来了
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

Transform * Transform::scale_ptr(Float x, Float y, Float z) {
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
    return new Transform(mat, matInv);
}


Transform * Transform::scale_ptr(Float s) {
    return Transform::scale_ptr(s, s, s);
}

Transform * Transform::scale_ptr(const Vector3f &s) {
    return Transform::scale_ptr(s.x, s.y, s.z);
}

Transform * Transform::translate_ptr(const Vector3f &delta) {
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
    return new Transform(mat, matInv);
}

Transform * Transform::rotateX_ptr(Float theta, bool bRadian/*=false*/) {
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
    return new Transform(mat, mat.getTransposeMat());
}

Transform * Transform::rotateY_ptr(Float theta, bool bRadian/*=false*/) {
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
    return new Transform(mat, mat.getTransposeMat());
}

Transform * Transform::rotateZ_ptr(Float theta, bool bRadian/*=false*/) {
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
    return new Transform(mat, mat.getTransposeMat());
}

Transform * Transform::rotate_ptr(Float theta, const Vector3f &axis, bool bRadian/*=false*/) {
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
   return new Transform(mat, mat.getTransposeMat());
}

Transform * Transform::lookAt_ptr(const Point3f &pos, const Point3f &look, const Vector3f &up) {
    //基本思路，先用up向量与dir向量确定right向量
    // right向量与dir向量互相垂直，由此可以确定新的up向量
    // right，dir，newUp向量两两垂直，可以构成直角坐标系，也就是视图空间
    Vector3f dir = normalize(look - pos);
    Vector3f right = cross(normalize(up), dir);
    if (right.lengthSquared() == 0) {
        // dir与up向量共线不合法
        DCHECK(false);
        return nullptr;
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
    return new Transform(cameraToWorld.getInverseMat(), cameraToWorld);
}

Transform * Transform::perspective_ptr(Float fov, Float zNear, Float zFar, bool bRadian/*=false*/) {
    //这里的透视矩阵没有aspect参数，是因为把光栅空间的变换分离出来了
    fov = bRadian ? fov : degree2radian(fov);
    Float invTanAng = 1 / std::tan(fov / 2);
    Float a[16] = {
        invTanAng, 0, 0,             0,
        0, invTanAng, 0,             0,
        0, 0, zFar / (zFar - zNear), -zFar * zNear / (zFar - zNear),
        0, 0,         1,             0
    };
    Matrix4x4 mat(a);
    return new Transform(mat);
}

Transform * Transform::identity_ptr() {
    Matrix4x4 mat;
    Matrix4x4 matInv;
    return new Transform(mat, matInv);
}

// 反射机制工厂函数
// 

/**
 * param : [x,y,z]
 */
CObject_ptr createScale(const nloJson &param, const Arguments &lst) {
    Float sx = param.at(0);
    Float sy = param.at(1);
    Float sz = param.at(2);
    return Transform::scale_ptr(sx, sy, sz);
}

/**
 * param : [x,y,z]
 */
CObject_ptr createTranslate(const nloJson &param, const Arguments &lst) {
    Float x = param.at(0);
    Float y = param.at(1);
    Float z = param.at(2);
    return Transform::translate_ptr(Vector3f(x, y, z));
}

/**
 * param : [theta, bRadian]
 */
CObject_ptr createRotateX(const nloJson &param, const Arguments &lst) {
    Float theta = param.at(0);
    bool bRadian = param.at(1);
    return Transform::rotateX_ptr(theta, bRadian);
}

/**
 * param : [theta, bRadian]
 */
CObject_ptr createRotateY(const nloJson &param, const Arguments &lst) {
    Float theta = param.at(0);
    bool bRadian = param.at(1);
    return Transform::rotateY_ptr(theta, bRadian);
}

/**
 * param : [theta, bRadian]
 */
CObject_ptr createRotateZ(const nloJson &param, const Arguments &lst) {
    Float theta = param.at(0);
    bool bRadian = param.at(1);
    return Transform::rotateZ_ptr(theta, bRadian);
}

/**
 * param : [
 *     theta,
 *     [x,y,z],
 *     bRadian
 * ]
 */
CObject_ptr createRotate(const nloJson &param, const Arguments &lst) {
   Float theta = param.value(0, 0);
   nloJson vec = param.at(1);
   Float ax = vec.at(0);
   Float ay = vec.at(1);
   Float az = vec.at(2);
   bool bRadian = param.at(2);
   Vector3f axis(ax, ay, az);
   return Transform::rotate_ptr(theta, axis, bRadian);
}

/**
 * param : [
 *     [x,y,z], // pos
 *     [x,y,z], // target
 *     [x,y,z], // up
 * ]
 */
CObject_ptr createLookAt(const nloJson &param, const Arguments &lst) {
    nloJson _pos = param.at(0); //nloJson::array({0, 0, -5})
    nloJson _target = param.at(1); //nloJson::array({0, 0, 0})
    nloJson _up = param.at(2); // nloJson::array({0, 1, 0})

    Float x = _pos.at(0);
    Float y = _pos.at(1);
    Float z = _pos.at(2);
    Point3f pos(x, y, z);

    x = _target.at(0);
    y = _target.at(1);
    z = _target.at(2);
    Point3f target(x, y, z);

    x = _up.at(0);
    y = _up.at(1);
    z = _up.at(2);
    Vector3f up(x, y, z);

    return Transform::lookAt_ptr(pos, target, up);
}

CObject_ptr createIdentity(const nloJson &, const Arguments &) {
    return Transform::identity_ptr();
}

//data : {
//    "type" : "translate",
//    "param" : [0,0,0]
//}
Transform * createTransform(const nloJson &data) {
    string type = data.value("type", "translate");
    auto creator = GET_CREATOR(type);
    nloJson param = data.value("param", nloJson::array({0, 0, 0}));
    Transform * ret = dynamic_cast<Transform *>(creator(param, {}));
    return ret;
}

REGISTER("scale", createScale);
REGISTER("translate", createTranslate);
REGISTER("rotateX", createRotateX);
REGISTER("rotateY", createRotateY);
REGISTER("rotateZ", createRotateZ);
REGISTER("rotate", createRotate);
REGISTER("lookAt", createLookAt);
REGISTER("identity", createIdentity);



PALADIN_END
