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

struct Matrix4x4 {
    Float m[4][4];
    
    Matrix4x4() {
        m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0;
        m[0][1] = m[0][2] = m[0][3] = 0.0f;
        m[1][0] = m[1][2] = m[1][3] = 0.0f;
        m[2][0] = m[2][1] = m[2][3] = 0.0f;
        m[3][0] = m[3][1] = m[3][2] = 0.0f;
    }
    
    Matrix4x4(Float mm[4][4]);
    
    Matrix4x4(Float t00, Float t01, Float t02, Float t03, Float t10, Float t11,
              Float t12, Float t13, Float t20, Float t21, Float t22, Float t23,
              Float t30, Float t31, Float t32, Float t33) {
        m[0][0] = t00;
        m[0][1] = t01;
        m[0][2] = t02;
        m[0][3] = t03;
        m[1][0] = t10;
        m[1][1] = t11;
        m[1][2] = t12;
        m[1][3] = t13;
        m[2][0] = t20;
        m[2][1] = t21;
        m[2][2] = t22;
        m[2][3] = t23;
        m[3][0] = t30;
        m[3][1] = t31;
        m[3][2] = t32;
        m[3][3] = t33;
    }
    
    bool operator==(const Matrix4x4& mat) const {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                if (m[i][j] != mat.m[i][j])
                    return false;
        return true;
    }
    
    bool operator!=(const Matrix4x4& mat) const {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                if (m[i][j] != mat.m[i][j])
                    return true;
        return false;
    }
    //转置矩阵
    friend Matrix4x4 transpose(const Matrix4x4 &mm);
    //逆矩阵
    friend Matrix4x4 inverse(const Matrix4x4 &mm);
    
    //矩阵相乘
    Matrix4x4 operator*(const Matrix4x4 &mm) const {
        Matrix4x4 mat;
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                mat.m[i][j] = m[i][0] * mm.m[0][j] + m[i][1] * mm.m[1][j]
                + m[i][2] * mm.m[2][j] + m[i][3] * mm.m[3][j];
        return mat;
    }
    
    //判断变量中是否包含nan分量
    bool hasNaNs() const {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                if (std::isnan(m[i][j]))
                    return true;
        return false;
    }
    
    friend std::ostream &operator<<(std::ostream &os, const Matrix4x4 &m) {
        os << "[[" << m.m[0][0] << " " << m.m[0][1] << " " << m.m[0][2] << " "
        << m.m[0][3] << "]" << " [" << m.m[1][0] << " " << m.m[1][1]
        << " " << m.m[1][2] << " " << m.m[1][3] << "]" << " ["
        << m.m[2][0] << " " << m.m[2][1] << " " << m.m[2][2] << " "
        << m.m[2][3] << "]" << " [" << m.m[3][0] << " " << m.m[3][1]
        << " " << m.m[3][2] << " " << m.m[3][3] << "]]";
        return os;
    }
    
    //    //矩阵相乘
    //    static Matrix4x4 Mul(const Matrix4x4 &mm1, const Matrix4x4 &mm2) {
    //        Matrix4x4 mat;
    //        for (int i = 0; i < 4; ++i)
    //            for (int j = 0; j < 4; ++j)
    //                mat.m[i][j] = mm1.m[i][0] * mm2.m[0][j]
    //                        + mm1.m[i][1] * mm2.m[1][j] + mm1.m[i][2] * mm2.m[2][j]
    //                        + mm1.m[i][3] * mm2.m[3][j];
    //        return mat;
    //    }
};

class Mat4 {
    
public:
    union {
        Float m[4][4];
        Float a[16];
        struct
        {
            Float _11; Float _12; Float _13; Float _14;
            Float _21; Float _22; Float _23; Float _24;
            Float _31; Float _32; Float _33; Float _34;
            Float _41; Float _42; Float _43; Float _44;
        };
    };
    
    Mat4() {
        for (int i = 0 ;i < 16 ; ++ i) {
            a[i] = 0;
        }
    }
    
    Mat4(Float p[4][4]) {
        for (int i = 0 ; i < 4; ++ i) {
            for (int j = 0 ; j < 4; ++ j) {
                m[i][j] = p[i][j];
            }
        }
    }
    
    Mat4(Float * value) {
        for (int i = 0 ;i< 16 ; ++ i) {
            a[i] = value[i];
        }
    }
    
    inline void setZero() {
        for (int i = 0 ; i < 16; ++ i) {
            a[i] = 0;
        }
    }
    
    // 伴随矩阵除以行列式
    Mat4 getInverseMat() const;
    
    Mat4 getTransposeMat() const;
    
    Mat4 getAdjointMat() const;
    
    Float getAdjointElement(Float a1, Float a2, Float a3,
                              Float b1, Float b2, Float b3,
                              Float c1, Float c2, Float c3)const;
    
    Float getDet() const;
    
    bool operator == (const Mat4 &other) const;
    
    Mat4 operator + (const Mat4 &other) const;
    
    Mat4 operator * (const Mat4 &other) const;
    
    Mat4 operator - (const Mat4 &other) const;
    
    Mat4 operator * (Float num) const;
    
    Mat4 operator / (Float num) const;
    
    static Mat4 identity();
    
};

PALADIN_END

#endif /* transform_hpp */
