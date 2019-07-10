//
//  transform.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef transform_hpp
#define transform_hpp

#include "paladin.hpp"

PALADIN_BEGIN


class Matrix4x4 {
    
public:
    
    Matrix4x4() {
        _m[0][0] = _m[1][1] = _m[2][2] = _m[3][3] = 1.f;
        _m[0][1] = _m[0][2] = _m[0][3] = _m[1][0] = _m[1][2] = _m[1][3] = _m[2][0] =
        _m[2][1] = _m[2][3] = _m[3][0] = _m[3][1] = _m[3][2] = 0.f;
    }
    
    Matrix4x4(const Float p[4][4]) {
        for (int i = 0 ; i < 4; ++ i) {
            for (int j = 0 ; j < 4; ++ j) {
                _m[i][j] = p[i][j];
            }
        }
    }
    
    Matrix4x4(const Float * value) {
        for (int i = 0 ;i< 16 ; ++ i) {
            _a[i] = value[i];
        }
    }
    
    inline void setZero() {
        for (int i = 0 ; i < 16; ++ i) {
            _a[i] = 0;
        }
    }
    
    // 伴随矩阵除以行列式
    Matrix4x4 getInverseMat() const;
    
    Matrix4x4 getTransposeMat() const;
    
    Matrix4x4 getAdjointMat() const;
    
    Float getAdjointElement(Float a1, Float a2, Float a3,
                              Float b1, Float b2, Float b3,
                              Float c1, Float c2, Float c3)const;
    
    Float getDet() const;
    
    //判断变量中是否包含nan分量
    bool hasNaNs() const {
        for (int i = 0; i < 16; ++i) {
            if (std::isnan(_a[i])) {
                return true;
            }
        }
        return false;
    }
    
    bool operator == (const Matrix4x4 &other) const;
    
    Matrix4x4 operator + (const Matrix4x4 &other) const;
    
    Matrix4x4 operator * (const Matrix4x4 &other) const;
    
    Matrix4x4 operator - (const Matrix4x4 &other) const;
    
    bool operator != (const Matrix4x4 &other) const;
    
    Matrix4x4 operator * (Float num) const;
    
    Matrix4x4 operator / (Float num) const;
    
    bool operator < (const Matrix4x4 &other) const;

    bool isIdentity() const;
    
    static Matrix4x4 identity();

    friend std::ostream &operator<<(std::ostream &os, const Matrix4x4 &mat) {
        // clang-format off
        os << StringPrintf("[ [ %f, %f, %f, %f ] "
                           "[ %f, %f, %f, %f ] "
                           "[ %f, %f, %f, %f ] "
                           "[ %f, %f, %f, %f ] ]",
                           mat._m[0][0], mat._m[0][1], mat._m[0][2], mat._m[0][3],
                           mat._m[1][0], mat._m[1][1], mat._m[1][2], mat._m[1][3],
                           mat._m[2][0], mat._m[2][1], mat._m[2][2], mat._m[2][3],
                           mat._m[3][0], mat._m[3][1], mat._m[3][2], mat._m[3][3]);
        // clang-format on
        return os;
    }
    
private:
    
    union {
        Float _m[4][4];
        Float _a[16];
        struct
        {
            Float _11; Float _12; Float _13; Float _14;
            Float _21; Float _22; Float _23; Float _24;
            Float _31; Float _32; Float _33; Float _34;
            Float _41; Float _42; Float _43; Float _44;
        };
    };

    friend class Transform;
};


class Transform {
// 参考pbrt设计变换类，包装了矩阵对象，只留变换接口，这样设计的好处在于，代码可读性高，
// 比之前在写OpenGL程序时，需要用齐次坐标来区分点与向量要清晰
public:
    Transform() {

    }

    Transform(const Float mat[4][4]) {
        _mat = Matrix4x4(mat);
        _matInv = _mat.getInverseMat();
    }

    Transform(const Matrix4x4 &mat) : _mat(mat), _matInv(_mat.getInverseMat()) {

    }

    Transform(const Matrix4x4 &mat, const Matrix4x4 &matInv) : _mat(mat), _matInv(matInv) {

    }

    Transform getInverse() const {
        return Transform(_matInv, _mat);
    }

    Transform getTranspose() const {
        return Transform(_mat.getTransposeMat(), _matInv.getTransposeMat());
    }

    bool operator == (const Transform &other) const {
        return other._mat == _mat && other._matInv == _matInv;
    }

    bool operator != (const Transform &other) const {
        return other._mat != _mat || other._matInv != _matInv;
    }

    bool operator < (const Transform &other) const {
        return _mat < other._mat;
    }

    bool isIdentity() const {
        return _mat.isIdentity();
    }

    const Matrix4x4 &getMatrix() const {
        return _mat;
    }

    const Matrix4x4 &getInverseMatrix() const {
        return _matInv;
    }

    // 对点执行转换
    template<typename T>
    inline Point3<T> exec(const Point3<T> &point) const {
        T x = point.x, y = point.y, z = point.z;
        T xp = _mat._m[0][0] * x + _mat._m[0][1] * y + _mat._m[0][2] * z + _mat._m[0][3];
        T yp = _mat._m[1][0] * x + _mat._m[1][1] * y + _mat._m[1][2] * z + _mat._m[1][3];
        T zp = _mat._m[2][0] * x + _mat._m[2][1] * y + _mat._m[2][2] * z + _mat._m[2][3];
        T wp = _mat._m[3][0] * x + _mat._m[3][1] * y + _mat._m[3][2] * z + _mat._m[3][3];
        CHECK_NE(wp, 0);
        if (wp == 1)
            return Point3<T>(xp, yp, zp);
        else
            return Point3<T>(xp, yp, zp) / wp;
    }

    template <typename T>
    inline Point3<T> exec(const Point3<T> &pt, Vector3<T> *absError) const;
    template <typename T>
    inline Point3<T> exec(const Point3<T> &p, const Vector3<T> &pError, Vector3<T> *pTransError) const;

    // 对向量执行转换
    template<typename T>
    inline Vector3<T> exec(const Vector3<T> &vec) const {
        T x = vec.x, y = vec.y, z = vec.z;
        return Vector3<T>(_mat._m[0][0] * x + _mat._m[0][1] * y + _mat._m[0][2] * z,
                          _mat._m[1][0] * x + _mat._m[1][1] * y + _mat._m[1][2] * z,
                          _mat._m[2][0] * x + _mat._m[2][1] * y + _mat._m[2][2] * z);
    }
    template <typename T>
    inline Vector3<T> exec(const Vector3<T> &v, Vector3<T> *vTransError) const;
    template <typename T>
    inline Vector3<T> exec(const Vector3<T> &v, const Vector3<T> &vError, Vector3<T> *vTransError) const;

    template<typename T>
    inline Normal3<T> exec(const Normal3<T> &normal) {
        /*
        法线的转换跟向量的转换所有不同，如果有非比例缩放法线就不能套用向量的转换了
        法线转换矩阵与原转换矩阵的关系如下t为切线向量,S为法线转换矩阵，M为切线转换矩阵
        dot(n, t) = 0
        transpose(n) * t = 0
        transpose(S * n) * (M * t) = 0
        transpose(n) * transpose(S) * M * t = 0 
        只要满足这个表达式的S矩阵，就是我们的目标矩阵
        我们可以假设 transpose(S) * M = I , 可得 S = transpose(inverse(M))
        */
        T x = normal.x, y = normal.y, z = normal.z;
        x = _matInv._m[0][0] * x + _matInv._m[1][0] * y + _matInv._m[2][0] * z;
        y = _matInv._m[0][1] * x + _matInv._m[1][1] * y + _matInv._m[2][1] * z;
        z = _matInv._m[0][2] * x + _matInv._m[1][2] * y + _matInv._m[2][2] * z;
        return Normal3<T>(x, y, z);
    }

    // 对射线执行转换
    inline Ray exec(const Ray &ray) const;
    inline Ray exec(const Ray &r, Vector3f *oError,
                          Vector3f *dError) const;
    inline Ray exec(const Ray &ray, const Vector3f &oErrorIn,
                          const Vector3f &dErrorIn, Vector3f *oErrorOut,
                          Vector3f *dErrorOut) const;


    inline RayDifferential exec(const RayDifferential &rd) const;

    Bounds3f exec(const Bounds3f &bounds) const;



    Transform operator * (const Transform &other) const;

    inline bool swapsHandedness() const {
        return _mat.getDet() < 0;
    }

    static Transform translate(const Vector3f &delta) {
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

    static Transform scale(Float x, Float y, Float z) {
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

    static Transform scale(Float s) {
        return Transform::scale(s, s, s);
    }

    static Transform rotateX(Float theta, bool bRadian=false) {
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

    static Transform rotateY(Float theta, bool bRadian=false) {
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

    static Transform rotateZ(Float theta, bool bRadian=false) {
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

//    static Transform rotate(Float theta, const Vector3f &axis, bool bRadian=false) {
////        Vector3f a = paladin::normalize(axis);
////        Vector3f a = paladin::normalize(axis);
//        theta = bRadian ? theta : degree2radian(theta);
//        Float sinTheta = std::sin(theta);
//        Float cosTheta = std::cos(theta);
//        Matrix4x4 mat;
//
//        mat._m[0][0] = a.x * a.x + (1 - a.x * a.x) * cosTheta;
//        mat._m[0][1] = a.x * a.y * (1 - cosTheta) - a.z * sinTheta;
//        mat._m[0][2] = a.x * a.z * (1 - cosTheta) + a.y * sinTheta;
//        mat._m[0][3] = 0;
//
//        mat._m[1][0] = a.x * a.y * (1 - cosTheta) + a.z * sinTheta;
//        mat._m[1][1] = a.y * a.y + (1 - a.y * a.y) * cosTheta;
//        mat._m[1][2] = a.y * a.z * (1 - cosTheta) - a.x * sinTheta;
//        mat._m[1][3] = 0;
//
//        mat._m[2][0] = a.x * a.z * (1 - cosTheta) - a.y * sinTheta;
//        mat._m[2][1] = a.y * a.z * (1 - cosTheta) + a.x * sinTheta;
//        mat._m[2][2] = a.z * a.z + (1 - a.z * a.z) * cosTheta;
//        mat._m[2][3] = 0;
//        // 旋转矩阵的逆矩阵为该矩阵的转置矩阵
//        return Transform(mat, mat.getTransposeMat());
//    }

//    static Transform lookAt(const Point3f &pos, const Point3f &look, const Vector3f &up) {
//        //基本思路，先用up向量与dir向量确定right向量
//        // right向量与dir向量互相垂直，由此可以确定新的up向量
//        // right，dir，newUp向量两两垂直，可以构成直角坐标系，也就是视图空间
//        Vector3f dir = normalize(look - pos);
//        Vector3f right = cross(normalize(up), dir);
//        if (right.lengthSquared() == 0) {
//            // dir与up向量共线不合法
//            return Transform();
//        }
//        right = normalize(right);
//        Vector3f newUp = cross(dir, right);
//        Float a[16] = {
//            right.x, newUp.x, dir.x, pos.x,
//            right.y, newUp.y, dir.y, pos.y,
//            right.z, newUp.z, dir.z, pos.z,
//            0,       0,       0,     1
//        };
//        Matrix4x4 cameraToWorld;
//        return Transform(cameraToWorld.getInverseMat(), cameraToWorld);
//    }

    static Transform orthographic(Float zNear, Float zFar) {
        Float a[16] = {
            1, 0, 0,                  0,
            0, 1, 0,                  0,
            0, 0, 1 / (zFar - zNear), -zNear,
            0, 0, 0,                  1,
        };
        return Transform(Matrix4x4(a));
    }

    static Transform perspective(Float fov, Float zNear, Float zFar, bool bRadian=false) {
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


private:

    Matrix4x4 _mat;
    Matrix4x4 _matInv;

};

PALADIN_END

#endif /* transform_hpp */
