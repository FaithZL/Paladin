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
        for (int i = 0 ;i < 16 ; ++ i) {
            _a[i] = 0;
        }
    }
    
    Matrix4x4(const Float p[4][4]) {
        for (int i = 0 ; i < 4; ++ i) {
            for (int j = 0 ; j < 4; ++ j) {
                m[i][j] = p[i][j];
            }
        }
    }
    
    Matrix4x4(Float * value) {
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
};

class Transform {
    
public:
    Transform() {
        
    }
    
    Transform(const Float mat[4][4]) {
        _mat = Matrix4x4(mat);
        _matInv = _mat.getInverseMat();
    }
    
    Transform(const Matrix4x4 &mat) : _mat(mat), _matInv(m.getInverseMat()) {
        
    }
    
    Transform(const Matrix4x4 &mat, const Matrix4x4 &matInv) : _mat(mat), matInv(_matInv) {
        
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
        return _matInv
    }

    template<typename T>
    inline Point3<T> exec(const Point3<T> &point) const;

    template<typename T>
    inline Vector3<T> exec(const Vector3<T> &vec) const;

    template<typename T>
    inline Normal3<T> exec(const Normal3<T> &normal) const;

    inline Ray exec(const Ray &ray) const;

    inline RayDifferential exec(const RayDifferential &rd) const;

    Bounds3f exec(const Bounds3f &bounds) const;

    Transform operator * (const Transform &other) const;

    bool swapsHandedness() const;
    
private:
    
    Matrix4x4 _mat;
    Matrix4x4 _matInv;
    
};

PALADIN_END

#endif /* transform_hpp */
