//
//  quaternion.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "quaternion.hpp"
#include "transform.hpp"
PALADIN_BEGIN

Quaternion slerp(Float t, const Quaternion &q1, const Quaternion &q2) {
    Float cosTheta = dot(q1, q2);
    if (cosTheta > .9995f)
        //如果旋转角度特别小，当做直线处理
        return normalize((1 - t) * q1 + t * q2);
    else {
        // 原始公式 result = (q1sin((1-t)θ) + q2sin(tθ)) / (sinθ)
        Float theta = std::acos(clamp(cosTheta, -1, 1));
        Float thetap = theta * t;
        Quaternion qperp = normalize(q2 - q1 * cosTheta);
        return q1 * std::cos(thetap) + qperp * std::sin(thetap);
    }
}

Transform Quaternion::ToTransform() const {
    //具体推导过程就不写了，比较复杂
    Float xx = v.x * v.x, yy = v.y * v.y, zz = v.z * v.z;
    Float xy = v.x * v.y, xz = v.x * v.z, yz = v.y * v.z;
    Float wx = v.x * w, wy = v.y * w, wz = v.z * w;
    Float a[16] = {
        1 - 2 * (yy + zz), 2 * (xy + wz),     2 * (xz - wy),     0,
        2 * (xy - wz),     1 - 2 * (xx + zz), 2 * (yz + wx),     0,
        2 * (xz + wy),     2 * (yz - wx),     1 - 2 * (xx + yy), 0,
        0,                 0,                 0,                 0
    };
    Matrix4x4 mat(a);
    return Transform(mat.getTransposeMat(), mat);
}

Quaternion::Quaternion(const Transform &t) {
    
}

PALADIN_END
