//
//  animatedtransform.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/7/13.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "animatedtransform.hpp"

PALADIN_BEGIN

AnimatedTransform::AnimatedTransform(const Transform *startTransform,
                                     Float startTime,
                                     const Transform *endTransform,
                                     Float endTime):
_startTransform(startTransform),
_endTransform(endTransform),
_startTime(startTime),
_endTime(endTime),
_actuallyAnimated(*startTransform != *endTransform){
    if (!_actuallyAnimated)
        return;
}

void AnimatedTransform::decompose(const Matrix4x4 &m, Vector3f *T,
                                  Quaternion *Rquat, Matrix4x4 *S) {
    //直接提取平移矩阵
    T->x = m._m[0][3];
    T->y = m._m[1][3];
    T->z = m._m[2][3];
    
    // 剔除旋转与缩放之外的分量
    Matrix4x4 M = m;
    for (int i = 0; i < 3; ++i) M._m[i][3] = M._m[3][i] = 0.f;
    M._m[3][3] = 1.f;
    
    // 用极分解提取旋转矩阵
    Float norm;
    int count = 0;
    Matrix4x4 R = M;
    do {
        // 计算下一个矩阵
        Matrix4x4 Rnext;
        Matrix4x4 Rit = R.getTransposeMat().getInverseMat();
        for (int i = 0; i < 16; ++i) {
            Rnext._a[i] = 0.5f * (R._a[i] + Rit._a[i]);
        }

        // 对比两个矩阵的差异，如果差异小于0.0001则分解完成
        norm = 0;
        for (int i = 0; i < 3; ++i) {
            Float n = std::abs(R._m[i][0] - Rnext._m[i][0]) +
            std::abs(R._m[i][1] - Rnext._m[i][1]) +
            std::abs(R._m[i][2] - Rnext._m[i][2]);
            norm = std::max(norm, n);
        }
        R = Rnext;
    } while (++count < 100 && norm > .0001);

    // XXX TODO FIXME deal with flip...
    *Rquat = Quaternion(R);
    
    *S = R.getInverseMat() * M;
}

PALADIN_END
