//
//  quaternion.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef quaternion_hpp
#define quaternion_hpp

#include "header.h"
PALADIN_BEGIN

struct Quaternion {
    /*
    跟复数一样，四元数用来表示旋转
    q = w + xi + yj + zk
    其中 i^2 = j^2 = k^2 = ijk = -1
    实部为w，虚部为x,y,z
    单位四元数为 x^2 + y^2 + z^2 + w^2 = 1

    四元数的乘法法则与复数相似
    qq' = (qw + qxi + qyj + qxk) * (q'w + q'xi + q'yj + q'xk)
    展开整理之后
    (qq')xyz = cross(qxyz, q'xyz) + qw * q'xyz + q'w * qxyz
    (qq')w = qw * q'w - dot(qxyz, q'xyz)

    四元数用法
    一个点p在绕某个向量单位v旋转2θ之后p',其中旋转四元数为q = (cosθ, v * sinθ)，q为单位四元数
    则满足p' = q * p * p^-1
    */
    Quaternion() : v(0, 0, 0), w(1) {}

    Quaternion &operator += (const Quaternion &q) {
        v += q.v;
        w += q.w;
        return *this;
    }

    friend Quaternion operator + (const Quaternion &q1, const Quaternion &q2) {
        Quaternion ret = q1;
        return ret += q2;
    }

    Quaternion &operator -= (const Quaternion &q) {
        v -= q.v;
        w -= q.w;
        return *this;
    }

    Quaternion operator - () const {
        Quaternion ret;
        ret.v = -v;
        ret.w = -w;
        return ret;
    }

    friend Quaternion operator - (const Quaternion &q1, const Quaternion &q2) {
        Quaternion ret = q1;
        return ret -= q2;
    }

    Quaternion &operator *= (Float f) {
        v *= f;
        w *= f;
        return *this;
    }

    Quaternion operator * (Float f) const {
        Quaternion ret = *this;
        ret.v *= f;
        ret.w *= f;
        return ret;
    }

    Quaternion &operator/=(Float f) {
        v /= f;
        w /= f;
        return *this;
    }

    Quaternion operator / (Float f) const {
        Quaternion ret = *this;
        ret.v /= f;
        ret.w /= f;
        return ret;
    }

    Transform ToTransform() const;

    Quaternion(const Transform &t);

    friend std::ostream &operator<<(std::ostream &os, const Quaternion &q) {
        os << StringPrintf("[ %f, %f, %f, %f ]", q.v.x, q.v.y, q.v.z,
                           q.w);
        return os;
    }
    
    Vector3f v;
    Float w;
};

inline Quaternion operator*(Float f, const Quaternion &q) { 
	return q * f; 
}

inline Float dot(const Quaternion &q1, const Quaternion &q2) {
    // 对应分量相乘之后相加
    return dot(q1.v, q2.v) + q1.w * q2.w;
}

inline Quaternion normalize(const Quaternion &q) {
    return q / std::sqrt(dot(q, q));
}

inline Quaternion slerp(Float t, const Quaternion &q1, const Quaternion &q2);

PALADIN_END

#endif /* quaternion_hpp */
