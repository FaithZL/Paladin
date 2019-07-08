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
            a[i] = 0;
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
            a[i] = value[i];
        }
    }
    
    inline void setZero() {
        for (int i = 0 ; i < 16; ++ i) {
            a[i] = 0;
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
            if (std::isnan(a[i])) {
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
    
    static Matrix4x4 identity();

    friend std::ostream &operator<<(std::ostream &os, const Matrix4x4 &m) {
        // clang-format off
        os << StringPrintf("[ [ %f, %f, %f, %f ] "
                           "[ %f, %f, %f, %f ] "
                           "[ %f, %f, %f, %f ] "
                           "[ %f, %f, %f, %f ] ]",
                           m.m[0][0], m.m[0][1], m.m[0][2], m.m[0][3],
                           m.m[1][0], m.m[1][1], m.m[1][2], m.m[1][3],
                           m.m[2][0], m.m[2][1], m.m[2][2], m.m[2][3],
                           m.m[3][0], m.m[3][1], m.m[3][2], m.m[3][3]);
        // clang-format on
        return os;
    }
    
private:
    
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
};

class Transform {
    
public:
    Transform() {
        
    }
    
    Transform(const Float mat[4][4]) {
        m = Matrix4x4(mat);
        mInv = m.getInverseMat();
    }
    
    Transform(const Matrix4x4 &m) : m(m), mInv(m.getInverseMat()) {
        
    }
    
    Transform(const Matrix4x4 &m, const Matrix4x4 &mInv) : m(m), mInv(mInv) {
        
    }
    
    Transform getInverse() const {
        return Transform(mInv, m);
    }
    
    Transform getTranspose() const {
        return Transform(m.getTransposeMat(), mInv.getTransposeMat());
    }
    
    bool operator == (const Transform &other) const {
        return other.m == m && other.mInv == mInv;
    }
    
    bool operator != (const Transform &other) const {
        return other.m != m || other.mInv != mInv;
    }
    
    bool operator < (const Transform &other) const {
        return m < other.m;
    }
    
private:
    
    Matrix4x4 m;
    Matrix4x4 mInv;
    
};

PALADIN_END

#endif /* transform_hpp */
