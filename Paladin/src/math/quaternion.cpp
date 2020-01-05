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
        // 比较直观的理解qperp = q2 - cosθ * q1 = q2 - dot(q1, q2) * q1
        // q' = q1 * cos(θt) + qperp * sin(θt)
        Float theta = std::acos(clamp(cosTheta, -1, 1));
        Float thetap = theta * t;
        Quaternion qperp = normalize(q2 - q1 * cosTheta);
        return q1 * std::cos(thetap) + qperp * std::sin(thetap);
    }
}

Transform Quaternion::toTransform() const {
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
    // 这个接口的推导过程没看，先搞完主线再说 todo
    const Matrix4x4 &m = t.getMatrix();
    Float trace = m._m[0][0] + m._m[1][1] + m._m[2][2];
    if (trace > 0.f) {
        // Compute w from matrix trace, then xyz
        // 4w^2 = m[0][0] + m[1][1] + m[2][2] + m[3][3] (but m[3][3] == 1)
        Float s = std::sqrt(trace + 1.0f);
        w = s / 2.0f;
        s = 0.5f / s;
        v.x = (m._m[2][1] - m._m[1][2]) * s;
        v.y = (m._m[0][2] - m._m[2][0]) * s;
        v.z = (m._m[1][0] - m._m[0][1]) * s;
    } else {
        // Compute largest of $x$, $y$, or $z$, then remaining components
        const int nxt[3] = {1, 2, 0};
        Float q[3];
        int i = 0;
        if (m._m[1][1] > m._m[0][0]) i = 1;
        if (m._m[2][2] > m._m[i][i]) i = 2;
        int j = nxt[i];
        int k = nxt[j];
        Float s = std::sqrt((m._m[i][i] - (m._m[j][j] + m._m[k][k])) + 1.0f);
        q[i] = s * 0.5f;
        if (s != 0.f) s = 0.5f / s;
        w = (m._m[k][j] - m._m[j][k]) * s;
        q[j] = (m._m[j][i] + m._m[i][j]) * s;
        q[k] = (m._m[k][i] + m._m[i][k]) * s;
        v.x = q[0];
        v.y = q[1];
        v.z = q[2];
    }
}

PALADIN_END
