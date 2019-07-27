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
    decompose(_startTransform->getMatrix(), &_T[0], &_R[0], &_S[0]);
    decompose(_endTransform->getMatrix(), &_T[1], &_R[1], &_S[1]);
    // 如果两个四元数的夹角大于90度，翻转第二个四元数，找到最短旋转路径
    // todo，关于四元数向量的问题，还不是很理解，搞完主线再说
    if (dot(_R[0], _R[1]) < 0)
        _R[1] = -_R[1];
    _hasRotation = dot(_R[0], _R[1]) < 0.9995f;
    // Compute terms of motion derivative function
    if (_hasRotation) {
        Float cosTheta = dot(_R[0], _R[1]);
        Float theta = std::acos(clamp(cosTheta, -1, 1));
        Quaternion qperp = normalize(_R[1] - _R[0] * cosTheta);
        
        Float t0x = _T[0].x;
        Float t0y = _T[0].y;
        Float t0z = _T[0].z;
        Float t1x = _T[1].x;
        Float t1y = _T[1].y;
        Float t1z = _T[1].z;
        Float q1x = _R[0].v.x;
        Float q1y = _R[0].v.y;
        Float q1z = _R[0].v.z;
        Float q1w = _R[0].w;
        Float qperpx = qperp.v.x;
        Float qperpy = qperp.v.y;
        Float qperpz = qperp.v.z;
        Float qperpw = qperp.w;
        Float s000 = _S[0]._m[0][0];
        Float s001 = _S[0]._m[0][1];
        Float s002 = _S[0]._m[0][2];
        Float s010 = _S[0]._m[1][0];
        Float s011 = _S[0]._m[1][1];
        Float s012 = _S[0]._m[1][2];
        Float s020 = _S[0]._m[2][0];
        Float s021 = _S[0]._m[2][1];
        Float s022 = _S[0]._m[2][2];
        Float s100 = _S[1]._m[0][0];
        Float s101 = _S[1]._m[0][1];
        Float s102 = _S[1]._m[0][2];
        Float s110 = _S[1]._m[1][0];
        Float s111 = _S[1]._m[1][1];
        Float s112 = _S[1]._m[1][2];
        Float s120 = _S[1]._m[2][0];
        Float s121 = _S[1]._m[2][1];
        Float s122 = _S[1]._m[2][2];
        
        c1[0] = DerivativeTerm(
                               -t0x + t1x,
                               (-1 + q1y * q1y + q1z * q1z + qperpy * qperpy + qperpz * qperpz) *
                               s000 +
                               q1w * q1z * s010 - qperpx * qperpy * s010 +
                               qperpw * qperpz * s010 - q1w * q1y * s020 -
                               qperpw * qperpy * s020 - qperpx * qperpz * s020 + s100 -
                               q1y * q1y * s100 - q1z * q1z * s100 - qperpy * qperpy * s100 -
                               qperpz * qperpz * s100 - q1w * q1z * s110 +
                               qperpx * qperpy * s110 - qperpw * qperpz * s110 +
                               q1w * q1y * s120 + qperpw * qperpy * s120 +
                               qperpx * qperpz * s120 +
                               q1x * (-(q1y * s010) - q1z * s020 + q1y * s110 + q1z * s120),
                               (-1 + q1y * q1y + q1z * q1z + qperpy * qperpy + qperpz * qperpz) *
                               s001 +
                               q1w * q1z * s011 - qperpx * qperpy * s011 +
                               qperpw * qperpz * s011 - q1w * q1y * s021 -
                               qperpw * qperpy * s021 - qperpx * qperpz * s021 + s101 -
                               q1y * q1y * s101 - q1z * q1z * s101 - qperpy * qperpy * s101 -
                               qperpz * qperpz * s101 - q1w * q1z * s111 +
                               qperpx * qperpy * s111 - qperpw * qperpz * s111 +
                               q1w * q1y * s121 + qperpw * qperpy * s121 +
                               qperpx * qperpz * s121 +
                               q1x * (-(q1y * s011) - q1z * s021 + q1y * s111 + q1z * s121),
                               (-1 + q1y * q1y + q1z * q1z + qperpy * qperpy + qperpz * qperpz) *
                               s002 +
                               q1w * q1z * s012 - qperpx * qperpy * s012 +
                               qperpw * qperpz * s012 - q1w * q1y * s022 -
                               qperpw * qperpy * s022 - qperpx * qperpz * s022 + s102 -
                               q1y * q1y * s102 - q1z * q1z * s102 - qperpy * qperpy * s102 -
                               qperpz * qperpz * s102 - q1w * q1z * s112 +
                               qperpx * qperpy * s112 - qperpw * qperpz * s112 +
                               q1w * q1y * s122 + qperpw * qperpy * s122 +
                               qperpx * qperpz * s122 +
                               q1x * (-(q1y * s012) - q1z * s022 + q1y * s112 + q1z * s122));
        
        c2[0] = DerivativeTerm(
                               0.,
                               -(qperpy * qperpy * s000) - qperpz * qperpz * s000 +
                               qperpx * qperpy * s010 - qperpw * qperpz * s010 +
                               qperpw * qperpy * s020 + qperpx * qperpz * s020 +
                               q1y * q1y * (s000 - s100) + q1z * q1z * (s000 - s100) +
                               qperpy * qperpy * s100 + qperpz * qperpz * s100 -
                               qperpx * qperpy * s110 + qperpw * qperpz * s110 -
                               qperpw * qperpy * s120 - qperpx * qperpz * s120 +
                               2 * q1x * qperpy * s010 * theta -
                               2 * q1w * qperpz * s010 * theta +
                               2 * q1w * qperpy * s020 * theta +
                               2 * q1x * qperpz * s020 * theta +
                               q1y *
                               (q1x * (-s010 + s110) + q1w * (-s020 + s120) +
                                2 * (-2 * qperpy * s000 + qperpx * s010 + qperpw * s020) *
                                theta) +
                               q1z * (q1w * (s010 - s110) + q1x * (-s020 + s120) -
                                      2 * (2 * qperpz * s000 + qperpw * s010 - qperpx * s020) *
                                      theta),
                               -(qperpy * qperpy * s001) - qperpz * qperpz * s001 +
                               qperpx * qperpy * s011 - qperpw * qperpz * s011 +
                               qperpw * qperpy * s021 + qperpx * qperpz * s021 +
                               q1y * q1y * (s001 - s101) + q1z * q1z * (s001 - s101) +
                               qperpy * qperpy * s101 + qperpz * qperpz * s101 -
                               qperpx * qperpy * s111 + qperpw * qperpz * s111 -
                               qperpw * qperpy * s121 - qperpx * qperpz * s121 +
                               2 * q1x * qperpy * s011 * theta -
                               2 * q1w * qperpz * s011 * theta +
                               2 * q1w * qperpy * s021 * theta +
                               2 * q1x * qperpz * s021 * theta +
                               q1y *
                               (q1x * (-s011 + s111) + q1w * (-s021 + s121) +
                                2 * (-2 * qperpy * s001 + qperpx * s011 + qperpw * s021) *
                                theta) +
                               q1z * (q1w * (s011 - s111) + q1x * (-s021 + s121) -
                                      2 * (2 * qperpz * s001 + qperpw * s011 - qperpx * s021) *
                                      theta),
                               -(qperpy * qperpy * s002) - qperpz * qperpz * s002 +
                               qperpx * qperpy * s012 - qperpw * qperpz * s012 +
                               qperpw * qperpy * s022 + qperpx * qperpz * s022 +
                               q1y * q1y * (s002 - s102) + q1z * q1z * (s002 - s102) +
                               qperpy * qperpy * s102 + qperpz * qperpz * s102 -
                               qperpx * qperpy * s112 + qperpw * qperpz * s112 -
                               qperpw * qperpy * s122 - qperpx * qperpz * s122 +
                               2 * q1x * qperpy * s012 * theta -
                               2 * q1w * qperpz * s012 * theta +
                               2 * q1w * qperpy * s022 * theta +
                               2 * q1x * qperpz * s022 * theta +
                               q1y *
                               (q1x * (-s012 + s112) + q1w * (-s022 + s122) +
                                2 * (-2 * qperpy * s002 + qperpx * s012 + qperpw * s022) *
                                theta) +
                               q1z * (q1w * (s012 - s112) + q1x * (-s022 + s122) -
                                      2 * (2 * qperpz * s002 + qperpw * s012 - qperpx * s022) *
                                      theta));
        
        c3[0] = DerivativeTerm(
                               0.,
                               -2 * (q1x * qperpy * s010 - q1w * qperpz * s010 +
                                     q1w * qperpy * s020 + q1x * qperpz * s020 -
                                     q1x * qperpy * s110 + q1w * qperpz * s110 -
                                     q1w * qperpy * s120 - q1x * qperpz * s120 +
                                     q1y * (-2 * qperpy * s000 + qperpx * s010 + qperpw * s020 +
                                            2 * qperpy * s100 - qperpx * s110 - qperpw * s120) +
                                     q1z * (-2 * qperpz * s000 - qperpw * s010 + qperpx * s020 +
                                            2 * qperpz * s100 + qperpw * s110 - qperpx * s120)) *
                               theta,
                               -2 * (q1x * qperpy * s011 - q1w * qperpz * s011 +
                                     q1w * qperpy * s021 + q1x * qperpz * s021 -
                                     q1x * qperpy * s111 + q1w * qperpz * s111 -
                                     q1w * qperpy * s121 - q1x * qperpz * s121 +
                                     q1y * (-2 * qperpy * s001 + qperpx * s011 + qperpw * s021 +
                                            2 * qperpy * s101 - qperpx * s111 - qperpw * s121) +
                                     q1z * (-2 * qperpz * s001 - qperpw * s011 + qperpx * s021 +
                                            2 * qperpz * s101 + qperpw * s111 - qperpx * s121)) *
                               theta,
                               -2 * (q1x * qperpy * s012 - q1w * qperpz * s012 +
                                     q1w * qperpy * s022 + q1x * qperpz * s022 -
                                     q1x * qperpy * s112 + q1w * qperpz * s112 -
                                     q1w * qperpy * s122 - q1x * qperpz * s122 +
                                     q1y * (-2 * qperpy * s002 + qperpx * s012 + qperpw * s022 +
                                            2 * qperpy * s102 - qperpx * s112 - qperpw * s122) +
                                     q1z * (-2 * qperpz * s002 - qperpw * s012 + qperpx * s022 +
                                            2 * qperpz * s102 + qperpw * s112 - qperpx * s122)) *
                               theta);
        
        c4[0] = DerivativeTerm(
                               0.,
                               -(q1x * qperpy * s010) + q1w * qperpz * s010 - q1w * qperpy * s020 -
                               q1x * qperpz * s020 + q1x * qperpy * s110 -
                               q1w * qperpz * s110 + q1w * qperpy * s120 +
                               q1x * qperpz * s120 + 2 * q1y * q1y * s000 * theta +
                               2 * q1z * q1z * s000 * theta -
                               2 * qperpy * qperpy * s000 * theta -
                               2 * qperpz * qperpz * s000 * theta +
                               2 * qperpx * qperpy * s010 * theta -
                               2 * qperpw * qperpz * s010 * theta +
                               2 * qperpw * qperpy * s020 * theta +
                               2 * qperpx * qperpz * s020 * theta +
                               q1y * (-(qperpx * s010) - qperpw * s020 +
                                      2 * qperpy * (s000 - s100) + qperpx * s110 +
                                      qperpw * s120 - 2 * q1x * s010 * theta -
                                      2 * q1w * s020 * theta) +
                               q1z * (2 * qperpz * s000 + qperpw * s010 - qperpx * s020 -
                                      2 * qperpz * s100 - qperpw * s110 + qperpx * s120 +
                                      2 * q1w * s010 * theta - 2 * q1x * s020 * theta),
                               -(q1x * qperpy * s011) + q1w * qperpz * s011 - q1w * qperpy * s021 -
                               q1x * qperpz * s021 + q1x * qperpy * s111 -
                               q1w * qperpz * s111 + q1w * qperpy * s121 +
                               q1x * qperpz * s121 + 2 * q1y * q1y * s001 * theta +
                               2 * q1z * q1z * s001 * theta -
                               2 * qperpy * qperpy * s001 * theta -
                               2 * qperpz * qperpz * s001 * theta +
                               2 * qperpx * qperpy * s011 * theta -
                               2 * qperpw * qperpz * s011 * theta +
                               2 * qperpw * qperpy * s021 * theta +
                               2 * qperpx * qperpz * s021 * theta +
                               q1y * (-(qperpx * s011) - qperpw * s021 +
                                      2 * qperpy * (s001 - s101) + qperpx * s111 +
                                      qperpw * s121 - 2 * q1x * s011 * theta -
                                      2 * q1w * s021 * theta) +
                               q1z * (2 * qperpz * s001 + qperpw * s011 - qperpx * s021 -
                                      2 * qperpz * s101 - qperpw * s111 + qperpx * s121 +
                                      2 * q1w * s011 * theta - 2 * q1x * s021 * theta),
                               -(q1x * qperpy * s012) + q1w * qperpz * s012 - q1w * qperpy * s022 -
                               q1x * qperpz * s022 + q1x * qperpy * s112 -
                               q1w * qperpz * s112 + q1w * qperpy * s122 +
                               q1x * qperpz * s122 + 2 * q1y * q1y * s002 * theta +
                               2 * q1z * q1z * s002 * theta -
                               2 * qperpy * qperpy * s002 * theta -
                               2 * qperpz * qperpz * s002 * theta +
                               2 * qperpx * qperpy * s012 * theta -
                               2 * qperpw * qperpz * s012 * theta +
                               2 * qperpw * qperpy * s022 * theta +
                               2 * qperpx * qperpz * s022 * theta +
                               q1y * (-(qperpx * s012) - qperpw * s022 +
                                      2 * qperpy * (s002 - s102) + qperpx * s112 +
                                      qperpw * s122 - 2 * q1x * s012 * theta -
                                      2 * q1w * s022 * theta) +
                               q1z * (2 * qperpz * s002 + qperpw * s012 - qperpx * s022 -
                                      2 * qperpz * s102 - qperpw * s112 + qperpx * s122 +
                                      2 * q1w * s012 * theta - 2 * q1x * s022 * theta));
        
        c5[0] = DerivativeTerm(
                               0.,
                               2 * (qperpy * qperpy * s000 + qperpz * qperpz * s000 -
                                    qperpx * qperpy * s010 + qperpw * qperpz * s010 -
                                    qperpw * qperpy * s020 - qperpx * qperpz * s020 -
                                    qperpy * qperpy * s100 - qperpz * qperpz * s100 +
                                    q1y * q1y * (-s000 + s100) + q1z * q1z * (-s000 + s100) +
                                    qperpx * qperpy * s110 - qperpw * qperpz * s110 +
                                    q1y * (q1x * (s010 - s110) + q1w * (s020 - s120)) +
                                    qperpw * qperpy * s120 + qperpx * qperpz * s120 +
                                    q1z * (-(q1w * s010) + q1x * s020 + q1w * s110 - q1x * s120)) *
                               theta,
                               2 * (qperpy * qperpy * s001 + qperpz * qperpz * s001 -
                                    qperpx * qperpy * s011 + qperpw * qperpz * s011 -
                                    qperpw * qperpy * s021 - qperpx * qperpz * s021 -
                                    qperpy * qperpy * s101 - qperpz * qperpz * s101 +
                                    q1y * q1y * (-s001 + s101) + q1z * q1z * (-s001 + s101) +
                                    qperpx * qperpy * s111 - qperpw * qperpz * s111 +
                                    q1y * (q1x * (s011 - s111) + q1w * (s021 - s121)) +
                                    qperpw * qperpy * s121 + qperpx * qperpz * s121 +
                                    q1z * (-(q1w * s011) + q1x * s021 + q1w * s111 - q1x * s121)) *
                               theta,
                               2 * (qperpy * qperpy * s002 + qperpz * qperpz * s002 -
                                    qperpx * qperpy * s012 + qperpw * qperpz * s012 -
                                    qperpw * qperpy * s022 - qperpx * qperpz * s022 -
                                    qperpy * qperpy * s102 - qperpz * qperpz * s102 +
                                    q1y * q1y * (-s002 + s102) + q1z * q1z * (-s002 + s102) +
                                    qperpx * qperpy * s112 - qperpw * qperpz * s112 +
                                    q1y * (q1x * (s012 - s112) + q1w * (s022 - s122)) +
                                    qperpw * qperpy * s122 + qperpx * qperpz * s122 +
                                    q1z * (-(q1w * s012) + q1x * s022 + q1w * s112 - q1x * s122)) *
                               theta);
        
        c1[1] = DerivativeTerm(
                               -t0y + t1y,
                               -(qperpx * qperpy * s000) - qperpw * qperpz * s000 - s010 +
                               q1z * q1z * s010 + qperpx * qperpx * s010 +
                               qperpz * qperpz * s010 - q1y * q1z * s020 +
                               qperpw * qperpx * s020 - qperpy * qperpz * s020 +
                               qperpx * qperpy * s100 + qperpw * qperpz * s100 +
                               q1w * q1z * (-s000 + s100) + q1x * q1x * (s010 - s110) + s110 -
                               q1z * q1z * s110 - qperpx * qperpx * s110 -
                               qperpz * qperpz * s110 +
                               q1x * (q1y * (-s000 + s100) + q1w * (s020 - s120)) +
                               q1y * q1z * s120 - qperpw * qperpx * s120 +
                               qperpy * qperpz * s120,
                               -(qperpx * qperpy * s001) - qperpw * qperpz * s001 - s011 +
                               q1z * q1z * s011 + qperpx * qperpx * s011 +
                               qperpz * qperpz * s011 - q1y * q1z * s021 +
                               qperpw * qperpx * s021 - qperpy * qperpz * s021 +
                               qperpx * qperpy * s101 + qperpw * qperpz * s101 +
                               q1w * q1z * (-s001 + s101) + q1x * q1x * (s011 - s111) + s111 -
                               q1z * q1z * s111 - qperpx * qperpx * s111 -
                               qperpz * qperpz * s111 +
                               q1x * (q1y * (-s001 + s101) + q1w * (s021 - s121)) +
                               q1y * q1z * s121 - qperpw * qperpx * s121 +
                               qperpy * qperpz * s121,
                               -(qperpx * qperpy * s002) - qperpw * qperpz * s002 - s012 +
                               q1z * q1z * s012 + qperpx * qperpx * s012 +
                               qperpz * qperpz * s012 - q1y * q1z * s022 +
                               qperpw * qperpx * s022 - qperpy * qperpz * s022 +
                               qperpx * qperpy * s102 + qperpw * qperpz * s102 +
                               q1w * q1z * (-s002 + s102) + q1x * q1x * (s012 - s112) + s112 -
                               q1z * q1z * s112 - qperpx * qperpx * s112 -
                               qperpz * qperpz * s112 +
                               q1x * (q1y * (-s002 + s102) + q1w * (s022 - s122)) +
                               q1y * q1z * s122 - qperpw * qperpx * s122 +
                               qperpy * qperpz * s122);
        
        c2[1] = DerivativeTerm(
                               0.,
                               qperpx * qperpy * s000 + qperpw * qperpz * s000 + q1z * q1z * s010 -
                               qperpx * qperpx * s010 - qperpz * qperpz * s010 -
                               q1y * q1z * s020 - qperpw * qperpx * s020 +
                               qperpy * qperpz * s020 - qperpx * qperpy * s100 -
                               qperpw * qperpz * s100 + q1x * q1x * (s010 - s110) -
                               q1z * q1z * s110 + qperpx * qperpx * s110 +
                               qperpz * qperpz * s110 + q1y * q1z * s120 +
                               qperpw * qperpx * s120 - qperpy * qperpz * s120 +
                               2 * q1z * qperpw * s000 * theta +
                               2 * q1y * qperpx * s000 * theta -
                               4 * q1z * qperpz * s010 * theta +
                               2 * q1z * qperpy * s020 * theta +
                               2 * q1y * qperpz * s020 * theta +
                               q1x * (q1w * s020 + q1y * (-s000 + s100) - q1w * s120 +
                                      2 * qperpy * s000 * theta - 4 * qperpx * s010 * theta -
                                      2 * qperpw * s020 * theta) +
                               q1w * (-(q1z * s000) + q1z * s100 + 2 * qperpz * s000 * theta -
                                      2 * qperpx * s020 * theta),
                               qperpx * qperpy * s001 + qperpw * qperpz * s001 + q1z * q1z * s011 -
                               qperpx * qperpx * s011 - qperpz * qperpz * s011 -
                               q1y * q1z * s021 - qperpw * qperpx * s021 +
                               qperpy * qperpz * s021 - qperpx * qperpy * s101 -
                               qperpw * qperpz * s101 + q1x * q1x * (s011 - s111) -
                               q1z * q1z * s111 + qperpx * qperpx * s111 +
                               qperpz * qperpz * s111 + q1y * q1z * s121 +
                               qperpw * qperpx * s121 - qperpy * qperpz * s121 +
                               2 * q1z * qperpw * s001 * theta +
                               2 * q1y * qperpx * s001 * theta -
                               4 * q1z * qperpz * s011 * theta +
                               2 * q1z * qperpy * s021 * theta +
                               2 * q1y * qperpz * s021 * theta +
                               q1x * (q1w * s021 + q1y * (-s001 + s101) - q1w * s121 +
                                      2 * qperpy * s001 * theta - 4 * qperpx * s011 * theta -
                                      2 * qperpw * s021 * theta) +
                               q1w * (-(q1z * s001) + q1z * s101 + 2 * qperpz * s001 * theta -
                                      2 * qperpx * s021 * theta),
                               qperpx * qperpy * s002 + qperpw * qperpz * s002 + q1z * q1z * s012 -
                               qperpx * qperpx * s012 - qperpz * qperpz * s012 -
                               q1y * q1z * s022 - qperpw * qperpx * s022 +
                               qperpy * qperpz * s022 - qperpx * qperpy * s102 -
                               qperpw * qperpz * s102 + q1x * q1x * (s012 - s112) -
                               q1z * q1z * s112 + qperpx * qperpx * s112 +
                               qperpz * qperpz * s112 + q1y * q1z * s122 +
                               qperpw * qperpx * s122 - qperpy * qperpz * s122 +
                               2 * q1z * qperpw * s002 * theta +
                               2 * q1y * qperpx * s002 * theta -
                               4 * q1z * qperpz * s012 * theta +
                               2 * q1z * qperpy * s022 * theta +
                               2 * q1y * qperpz * s022 * theta +
                               q1x * (q1w * s022 + q1y * (-s002 + s102) - q1w * s122 +
                                      2 * qperpy * s002 * theta - 4 * qperpx * s012 * theta -
                                      2 * qperpw * s022 * theta) +
                               q1w * (-(q1z * s002) + q1z * s102 + 2 * qperpz * s002 * theta -
                                      2 * qperpx * s022 * theta));
        
        c3[1] = DerivativeTerm(
                               0., 2 * (-(q1x * qperpy * s000) - q1w * qperpz * s000 +
                                        2 * q1x * qperpx * s010 + q1x * qperpw * s020 +
                                        q1w * qperpx * s020 + q1x * qperpy * s100 +
                                        q1w * qperpz * s100 - 2 * q1x * qperpx * s110 -
                                        q1x * qperpw * s120 - q1w * qperpx * s120 +
                                        q1z * (2 * qperpz * s010 - qperpy * s020 +
                                               qperpw * (-s000 + s100) - 2 * qperpz * s110 +
                                               qperpy * s120) +
                                        q1y * (-(qperpx * s000) - qperpz * s020 + qperpx * s100 +
                                               qperpz * s120)) *
                               theta,
                               2 * (-(q1x * qperpy * s001) - q1w * qperpz * s001 +
                                    2 * q1x * qperpx * s011 + q1x * qperpw * s021 +
                                    q1w * qperpx * s021 + q1x * qperpy * s101 +
                                    q1w * qperpz * s101 - 2 * q1x * qperpx * s111 -
                                    q1x * qperpw * s121 - q1w * qperpx * s121 +
                                    q1z * (2 * qperpz * s011 - qperpy * s021 +
                                           qperpw * (-s001 + s101) - 2 * qperpz * s111 +
                                           qperpy * s121) +
                                    q1y * (-(qperpx * s001) - qperpz * s021 + qperpx * s101 +
                                           qperpz * s121)) *
                               theta,
                               2 * (-(q1x * qperpy * s002) - q1w * qperpz * s002 +
                                    2 * q1x * qperpx * s012 + q1x * qperpw * s022 +
                                    q1w * qperpx * s022 + q1x * qperpy * s102 +
                                    q1w * qperpz * s102 - 2 * q1x * qperpx * s112 -
                                    q1x * qperpw * s122 - q1w * qperpx * s122 +
                                    q1z * (2 * qperpz * s012 - qperpy * s022 +
                                           qperpw * (-s002 + s102) - 2 * qperpz * s112 +
                                           qperpy * s122) +
                                    q1y * (-(qperpx * s002) - qperpz * s022 + qperpx * s102 +
                                           qperpz * s122)) *
                               theta);
        
        c4[1] = DerivativeTerm(
                               0.,
                               -(q1x * qperpy * s000) - q1w * qperpz * s000 +
                               2 * q1x * qperpx * s010 + q1x * qperpw * s020 +
                               q1w * qperpx * s020 + q1x * qperpy * s100 +
                               q1w * qperpz * s100 - 2 * q1x * qperpx * s110 -
                               q1x * qperpw * s120 - q1w * qperpx * s120 +
                               2 * qperpx * qperpy * s000 * theta +
                               2 * qperpw * qperpz * s000 * theta +
                               2 * q1x * q1x * s010 * theta + 2 * q1z * q1z * s010 * theta -
                               2 * qperpx * qperpx * s010 * theta -
                               2 * qperpz * qperpz * s010 * theta +
                               2 * q1w * q1x * s020 * theta -
                               2 * qperpw * qperpx * s020 * theta +
                               2 * qperpy * qperpz * s020 * theta +
                               q1y * (-(qperpx * s000) - qperpz * s020 + qperpx * s100 +
                                      qperpz * s120 - 2 * q1x * s000 * theta) +
                               q1z * (2 * qperpz * s010 - qperpy * s020 +
                                      qperpw * (-s000 + s100) - 2 * qperpz * s110 +
                                      qperpy * s120 - 2 * q1w * s000 * theta -
                                      2 * q1y * s020 * theta),
                               -(q1x * qperpy * s001) - q1w * qperpz * s001 +
                               2 * q1x * qperpx * s011 + q1x * qperpw * s021 +
                               q1w * qperpx * s021 + q1x * qperpy * s101 +
                               q1w * qperpz * s101 - 2 * q1x * qperpx * s111 -
                               q1x * qperpw * s121 - q1w * qperpx * s121 +
                               2 * qperpx * qperpy * s001 * theta +
                               2 * qperpw * qperpz * s001 * theta +
                               2 * q1x * q1x * s011 * theta + 2 * q1z * q1z * s011 * theta -
                               2 * qperpx * qperpx * s011 * theta -
                               2 * qperpz * qperpz * s011 * theta +
                               2 * q1w * q1x * s021 * theta -
                               2 * qperpw * qperpx * s021 * theta +
                               2 * qperpy * qperpz * s021 * theta +
                               q1y * (-(qperpx * s001) - qperpz * s021 + qperpx * s101 +
                                      qperpz * s121 - 2 * q1x * s001 * theta) +
                               q1z * (2 * qperpz * s011 - qperpy * s021 +
                                      qperpw * (-s001 + s101) - 2 * qperpz * s111 +
                                      qperpy * s121 - 2 * q1w * s001 * theta -
                                      2 * q1y * s021 * theta),
                               -(q1x * qperpy * s002) - q1w * qperpz * s002 +
                               2 * q1x * qperpx * s012 + q1x * qperpw * s022 +
                               q1w * qperpx * s022 + q1x * qperpy * s102 +
                               q1w * qperpz * s102 - 2 * q1x * qperpx * s112 -
                               q1x * qperpw * s122 - q1w * qperpx * s122 +
                               2 * qperpx * qperpy * s002 * theta +
                               2 * qperpw * qperpz * s002 * theta +
                               2 * q1x * q1x * s012 * theta + 2 * q1z * q1z * s012 * theta -
                               2 * qperpx * qperpx * s012 * theta -
                               2 * qperpz * qperpz * s012 * theta +
                               2 * q1w * q1x * s022 * theta -
                               2 * qperpw * qperpx * s022 * theta +
                               2 * qperpy * qperpz * s022 * theta +
                               q1y * (-(qperpx * s002) - qperpz * s022 + qperpx * s102 +
                                      qperpz * s122 - 2 * q1x * s002 * theta) +
                               q1z * (2 * qperpz * s012 - qperpy * s022 +
                                      qperpw * (-s002 + s102) - 2 * qperpz * s112 +
                                      qperpy * s122 - 2 * q1w * s002 * theta -
                                      2 * q1y * s022 * theta));
        
        c5[1] = DerivativeTerm(
                               0., -2 * (qperpx * qperpy * s000 + qperpw * qperpz * s000 +
                                         q1z * q1z * s010 - qperpx * qperpx * s010 -
                                         qperpz * qperpz * s010 - q1y * q1z * s020 -
                                         qperpw * qperpx * s020 + qperpy * qperpz * s020 -
                                         qperpx * qperpy * s100 - qperpw * qperpz * s100 +
                                         q1w * q1z * (-s000 + s100) + q1x * q1x * (s010 - s110) -
                                         q1z * q1z * s110 + qperpx * qperpx * s110 +
                                         qperpz * qperpz * s110 +
                                         q1x * (q1y * (-s000 + s100) + q1w * (s020 - s120)) +
                                         q1y * q1z * s120 + qperpw * qperpx * s120 -
                                         qperpy * qperpz * s120) *
                               theta,
                               -2 * (qperpx * qperpy * s001 + qperpw * qperpz * s001 +
                                     q1z * q1z * s011 - qperpx * qperpx * s011 -
                                     qperpz * qperpz * s011 - q1y * q1z * s021 -
                                     qperpw * qperpx * s021 + qperpy * qperpz * s021 -
                                     qperpx * qperpy * s101 - qperpw * qperpz * s101 +
                                     q1w * q1z * (-s001 + s101) + q1x * q1x * (s011 - s111) -
                                     q1z * q1z * s111 + qperpx * qperpx * s111 +
                                     qperpz * qperpz * s111 +
                                     q1x * (q1y * (-s001 + s101) + q1w * (s021 - s121)) +
                                     q1y * q1z * s121 + qperpw * qperpx * s121 -
                                     qperpy * qperpz * s121) *
                               theta,
                               -2 * (qperpx * qperpy * s002 + qperpw * qperpz * s002 +
                                     q1z * q1z * s012 - qperpx * qperpx * s012 -
                                     qperpz * qperpz * s012 - q1y * q1z * s022 -
                                     qperpw * qperpx * s022 + qperpy * qperpz * s022 -
                                     qperpx * qperpy * s102 - qperpw * qperpz * s102 +
                                     q1w * q1z * (-s002 + s102) + q1x * q1x * (s012 - s112) -
                                     q1z * q1z * s112 + qperpx * qperpx * s112 +
                                     qperpz * qperpz * s112 +
                                     q1x * (q1y * (-s002 + s102) + q1w * (s022 - s122)) +
                                     q1y * q1z * s122 + qperpw * qperpx * s122 -
                                     qperpy * qperpz * s122) *
                               theta);
        
        c1[2] = DerivativeTerm(
                               -t0z + t1z, (qperpw * qperpy * s000 - qperpx * qperpz * s000 -
                                            q1y * q1z * s010 - qperpw * qperpx * s010 -
                                            qperpy * qperpz * s010 - s020 + q1y * q1y * s020 +
                                            qperpx * qperpx * s020 + qperpy * qperpy * s020 -
                                            qperpw * qperpy * s100 + qperpx * qperpz * s100 +
                                            q1x * q1z * (-s000 + s100) + q1y * q1z * s110 +
                                            qperpw * qperpx * s110 + qperpy * qperpz * s110 +
                                            q1w * (q1y * (s000 - s100) + q1x * (-s010 + s110)) +
                                            q1x * q1x * (s020 - s120) + s120 - q1y * q1y * s120 -
                                            qperpx * qperpx * s120 - qperpy * qperpy * s120),
                               (qperpw * qperpy * s001 - qperpx * qperpz * s001 -
                                q1y * q1z * s011 - qperpw * qperpx * s011 -
                                qperpy * qperpz * s011 - s021 + q1y * q1y * s021 +
                                qperpx * qperpx * s021 + qperpy * qperpy * s021 -
                                qperpw * qperpy * s101 + qperpx * qperpz * s101 +
                                q1x * q1z * (-s001 + s101) + q1y * q1z * s111 +
                                qperpw * qperpx * s111 + qperpy * qperpz * s111 +
                                q1w * (q1y * (s001 - s101) + q1x * (-s011 + s111)) +
                                q1x * q1x * (s021 - s121) + s121 - q1y * q1y * s121 -
                                qperpx * qperpx * s121 - qperpy * qperpy * s121),
                               (qperpw * qperpy * s002 - qperpx * qperpz * s002 -
                                q1y * q1z * s012 - qperpw * qperpx * s012 -
                                qperpy * qperpz * s012 - s022 + q1y * q1y * s022 +
                                qperpx * qperpx * s022 + qperpy * qperpy * s022 -
                                qperpw * qperpy * s102 + qperpx * qperpz * s102 +
                                q1x * q1z * (-s002 + s102) + q1y * q1z * s112 +
                                qperpw * qperpx * s112 + qperpy * qperpz * s112 +
                                q1w * (q1y * (s002 - s102) + q1x * (-s012 + s112)) +
                                q1x * q1x * (s022 - s122) + s122 - q1y * q1y * s122 -
                                qperpx * qperpx * s122 - qperpy * qperpy * s122));
        
        c2[2] = DerivativeTerm(
                               0.,
                               (q1w * q1y * s000 - q1x * q1z * s000 - qperpw * qperpy * s000 +
                                qperpx * qperpz * s000 - q1w * q1x * s010 - q1y * q1z * s010 +
                                qperpw * qperpx * s010 + qperpy * qperpz * s010 +
                                q1x * q1x * s020 + q1y * q1y * s020 - qperpx * qperpx * s020 -
                                qperpy * qperpy * s020 - q1w * q1y * s100 + q1x * q1z * s100 +
                                qperpw * qperpy * s100 - qperpx * qperpz * s100 +
                                q1w * q1x * s110 + q1y * q1z * s110 - qperpw * qperpx * s110 -
                                qperpy * qperpz * s110 - q1x * q1x * s120 - q1y * q1y * s120 +
                                qperpx * qperpx * s120 + qperpy * qperpy * s120 -
                                2 * q1y * qperpw * s000 * theta + 2 * q1z * qperpx * s000 * theta -
                                2 * q1w * qperpy * s000 * theta + 2 * q1x * qperpz * s000 * theta +
                                2 * q1x * qperpw * s010 * theta + 2 * q1w * qperpx * s010 * theta +
                                2 * q1z * qperpy * s010 * theta + 2 * q1y * qperpz * s010 * theta -
                                4 * q1x * qperpx * s020 * theta - 4 * q1y * qperpy * s020 * theta),
                               (q1w * q1y * s001 - q1x * q1z * s001 - qperpw * qperpy * s001 +
                                qperpx * qperpz * s001 - q1w * q1x * s011 - q1y * q1z * s011 +
                                qperpw * qperpx * s011 + qperpy * qperpz * s011 +
                                q1x * q1x * s021 + q1y * q1y * s021 - qperpx * qperpx * s021 -
                                qperpy * qperpy * s021 - q1w * q1y * s101 + q1x * q1z * s101 +
                                qperpw * qperpy * s101 - qperpx * qperpz * s101 +
                                q1w * q1x * s111 + q1y * q1z * s111 - qperpw * qperpx * s111 -
                                qperpy * qperpz * s111 - q1x * q1x * s121 - q1y * q1y * s121 +
                                qperpx * qperpx * s121 + qperpy * qperpy * s121 -
                                2 * q1y * qperpw * s001 * theta + 2 * q1z * qperpx * s001 * theta -
                                2 * q1w * qperpy * s001 * theta + 2 * q1x * qperpz * s001 * theta +
                                2 * q1x * qperpw * s011 * theta + 2 * q1w * qperpx * s011 * theta +
                                2 * q1z * qperpy * s011 * theta + 2 * q1y * qperpz * s011 * theta -
                                4 * q1x * qperpx * s021 * theta - 4 * q1y * qperpy * s021 * theta),
                               (q1w * q1y * s002 - q1x * q1z * s002 - qperpw * qperpy * s002 +
                                qperpx * qperpz * s002 - q1w * q1x * s012 - q1y * q1z * s012 +
                                qperpw * qperpx * s012 + qperpy * qperpz * s012 +
                                q1x * q1x * s022 + q1y * q1y * s022 - qperpx * qperpx * s022 -
                                qperpy * qperpy * s022 - q1w * q1y * s102 + q1x * q1z * s102 +
                                qperpw * qperpy * s102 - qperpx * qperpz * s102 +
                                q1w * q1x * s112 + q1y * q1z * s112 - qperpw * qperpx * s112 -
                                qperpy * qperpz * s112 - q1x * q1x * s122 - q1y * q1y * s122 +
                                qperpx * qperpx * s122 + qperpy * qperpy * s122 -
                                2 * q1y * qperpw * s002 * theta + 2 * q1z * qperpx * s002 * theta -
                                2 * q1w * qperpy * s002 * theta + 2 * q1x * qperpz * s002 * theta +
                                2 * q1x * qperpw * s012 * theta + 2 * q1w * qperpx * s012 * theta +
                                2 * q1z * qperpy * s012 * theta + 2 * q1y * qperpz * s012 * theta -
                                4 * q1x * qperpx * s022 * theta -
                                4 * q1y * qperpy * s022 * theta));
        
        c3[2] = DerivativeTerm(
                               0., -2 * (-(q1w * qperpy * s000) + q1x * qperpz * s000 +
                                         q1x * qperpw * s010 + q1w * qperpx * s010 -
                                         2 * q1x * qperpx * s020 + q1w * qperpy * s100 -
                                         q1x * qperpz * s100 - q1x * qperpw * s110 -
                                         q1w * qperpx * s110 +
                                         q1z * (qperpx * s000 + qperpy * s010 - qperpx * s100 -
                                                qperpy * s110) +
                                         2 * q1x * qperpx * s120 +
                                         q1y * (qperpz * s010 - 2 * qperpy * s020 +
                                                qperpw * (-s000 + s100) - qperpz * s110 +
                                                2 * qperpy * s120)) *
                               theta,
                               -2 * (-(q1w * qperpy * s001) + q1x * qperpz * s001 +
                                     q1x * qperpw * s011 + q1w * qperpx * s011 -
                                     2 * q1x * qperpx * s021 + q1w * qperpy * s101 -
                                     q1x * qperpz * s101 - q1x * qperpw * s111 -
                                     q1w * qperpx * s111 +
                                     q1z * (qperpx * s001 + qperpy * s011 - qperpx * s101 -
                                            qperpy * s111) +
                                     2 * q1x * qperpx * s121 +
                                     q1y * (qperpz * s011 - 2 * qperpy * s021 +
                                            qperpw * (-s001 + s101) - qperpz * s111 +
                                            2 * qperpy * s121)) *
                               theta,
                               -2 * (-(q1w * qperpy * s002) + q1x * qperpz * s002 +
                                     q1x * qperpw * s012 + q1w * qperpx * s012 -
                                     2 * q1x * qperpx * s022 + q1w * qperpy * s102 -
                                     q1x * qperpz * s102 - q1x * qperpw * s112 -
                                     q1w * qperpx * s112 +
                                     q1z * (qperpx * s002 + qperpy * s012 - qperpx * s102 -
                                            qperpy * s112) +
                                     2 * q1x * qperpx * s122 +
                                     q1y * (qperpz * s012 - 2 * qperpy * s022 +
                                            qperpw * (-s002 + s102) - qperpz * s112 +
                                            2 * qperpy * s122)) *
                               theta);
        
        c4[2] = DerivativeTerm(
                               0.,
                               q1w * qperpy * s000 - q1x * qperpz * s000 - q1x * qperpw * s010 -
                               q1w * qperpx * s010 + 2 * q1x * qperpx * s020 -
                               q1w * qperpy * s100 + q1x * qperpz * s100 +
                               q1x * qperpw * s110 + q1w * qperpx * s110 -
                               2 * q1x * qperpx * s120 - 2 * qperpw * qperpy * s000 * theta +
                               2 * qperpx * qperpz * s000 * theta -
                               2 * q1w * q1x * s010 * theta +
                               2 * qperpw * qperpx * s010 * theta +
                               2 * qperpy * qperpz * s010 * theta +
                               2 * q1x * q1x * s020 * theta + 2 * q1y * q1y * s020 * theta -
                               2 * qperpx * qperpx * s020 * theta -
                               2 * qperpy * qperpy * s020 * theta +
                               q1z * (-(qperpx * s000) - qperpy * s010 + qperpx * s100 +
                                      qperpy * s110 - 2 * q1x * s000 * theta) +
                               q1y * (-(qperpz * s010) + 2 * qperpy * s020 +
                                      qperpw * (s000 - s100) + qperpz * s110 -
                                      2 * qperpy * s120 + 2 * q1w * s000 * theta -
                                      2 * q1z * s010 * theta),
                               q1w * qperpy * s001 - q1x * qperpz * s001 - q1x * qperpw * s011 -
                               q1w * qperpx * s011 + 2 * q1x * qperpx * s021 -
                               q1w * qperpy * s101 + q1x * qperpz * s101 +
                               q1x * qperpw * s111 + q1w * qperpx * s111 -
                               2 * q1x * qperpx * s121 - 2 * qperpw * qperpy * s001 * theta +
                               2 * qperpx * qperpz * s001 * theta -
                               2 * q1w * q1x * s011 * theta +
                               2 * qperpw * qperpx * s011 * theta +
                               2 * qperpy * qperpz * s011 * theta +
                               2 * q1x * q1x * s021 * theta + 2 * q1y * q1y * s021 * theta -
                               2 * qperpx * qperpx * s021 * theta -
                               2 * qperpy * qperpy * s021 * theta +
                               q1z * (-(qperpx * s001) - qperpy * s011 + qperpx * s101 +
                                      qperpy * s111 - 2 * q1x * s001 * theta) +
                               q1y * (-(qperpz * s011) + 2 * qperpy * s021 +
                                      qperpw * (s001 - s101) + qperpz * s111 -
                                      2 * qperpy * s121 + 2 * q1w * s001 * theta -
                                      2 * q1z * s011 * theta),
                               q1w * qperpy * s002 - q1x * qperpz * s002 - q1x * qperpw * s012 -
                               q1w * qperpx * s012 + 2 * q1x * qperpx * s022 -
                               q1w * qperpy * s102 + q1x * qperpz * s102 +
                               q1x * qperpw * s112 + q1w * qperpx * s112 -
                               2 * q1x * qperpx * s122 - 2 * qperpw * qperpy * s002 * theta +
                               2 * qperpx * qperpz * s002 * theta -
                               2 * q1w * q1x * s012 * theta +
                               2 * qperpw * qperpx * s012 * theta +
                               2 * qperpy * qperpz * s012 * theta +
                               2 * q1x * q1x * s022 * theta + 2 * q1y * q1y * s022 * theta -
                               2 * qperpx * qperpx * s022 * theta -
                               2 * qperpy * qperpy * s022 * theta +
                               q1z * (-(qperpx * s002) - qperpy * s012 + qperpx * s102 +
                                      qperpy * s112 - 2 * q1x * s002 * theta) +
                               q1y * (-(qperpz * s012) + 2 * qperpy * s022 +
                                      qperpw * (s002 - s102) + qperpz * s112 -
                                      2 * qperpy * s122 + 2 * q1w * s002 * theta -
                                      2 * q1z * s012 * theta));
        
        c5[2] = DerivativeTerm(
                               0., 2 * (qperpw * qperpy * s000 - qperpx * qperpz * s000 +
                                        q1y * q1z * s010 - qperpw * qperpx * s010 -
                                        qperpy * qperpz * s010 - q1y * q1y * s020 +
                                        qperpx * qperpx * s020 + qperpy * qperpy * s020 +
                                        q1x * q1z * (s000 - s100) - qperpw * qperpy * s100 +
                                        qperpx * qperpz * s100 +
                                        q1w * (q1y * (-s000 + s100) + q1x * (s010 - s110)) -
                                        q1y * q1z * s110 + qperpw * qperpx * s110 +
                                        qperpy * qperpz * s110 + q1y * q1y * s120 -
                                        qperpx * qperpx * s120 - qperpy * qperpy * s120 +
                                        q1x * q1x * (-s020 + s120)) *
                               theta,
                               2 * (qperpw * qperpy * s001 - qperpx * qperpz * s001 +
                                    q1y * q1z * s011 - qperpw * qperpx * s011 -
                                    qperpy * qperpz * s011 - q1y * q1y * s021 +
                                    qperpx * qperpx * s021 + qperpy * qperpy * s021 +
                                    q1x * q1z * (s001 - s101) - qperpw * qperpy * s101 +
                                    qperpx * qperpz * s101 +
                                    q1w * (q1y * (-s001 + s101) + q1x * (s011 - s111)) -
                                    q1y * q1z * s111 + qperpw * qperpx * s111 +
                                    qperpy * qperpz * s111 + q1y * q1y * s121 -
                                    qperpx * qperpx * s121 - qperpy * qperpy * s121 +
                                    q1x * q1x * (-s021 + s121)) *
                               theta,
                               2 * (qperpw * qperpy * s002 - qperpx * qperpz * s002 +
                                    q1y * q1z * s012 - qperpw * qperpx * s012 -
                                    qperpy * qperpz * s012 - q1y * q1y * s022 +
                                    qperpx * qperpx * s022 + qperpy * qperpy * s022 +
                                    q1x * q1z * (s002 - s102) - qperpw * qperpy * s102 +
                                    qperpx * qperpz * s102 +
                                    q1w * (q1y * (-s002 + s102) + q1x * (s012 - s112)) -
                                    q1y * q1z * s112 + qperpw * qperpx * s112 +
                                    qperpy * qperpz * s112 + q1y * q1y * s122 -
                                    qperpx * qperpx * s122 - qperpy * qperpy * s122 +
                                    q1x * q1x * (-s022 + s122)) *
                               theta);
    }
}

Bounds3f AnimatedTransform::MotionBounds(const Bounds3f &b) const {
    if (!_actuallyAnimated)
        // 如果没有变化，直接使用变换
        return _startTransform->exec(b);
    if (_hasRotation == false)
        // 如果没有旋转，只有平移缩放，可以用两个变换的并集
        // 因为没有只有平移缩放的情况下 p'= f(t) * p，p'与t呈线性关系
        return unionSet(_startTransform->exec(b), _endTransform->exec(b));
    // 如果有旋转，则用最暴力的方式，计算8个顶点轨迹的包围盒，然后取并集
    Bounds3f bounds;
    for (int corner = 0; corner < 8; ++corner)
        bounds = unionSet(bounds, BoundPointMotion(b.corner(corner)));
    return bounds;
}

void AnimatedTransform::interpolate(Float time, Transform *t) const {
    *t = interpolate(time);
}

Ray AnimatedTransform::exec(const paladin::Ray &ray) const {
    if (!_actuallyAnimated || ray.time <= _startTime) {
        return _startTransform->exec(ray);
    } else if (ray.time >= _endTime) {
        return _endTransform->exec(ray);
    }
    Transform trsf = interpolate(ray.time);
    return trsf.exec(ray);
}

RayDifferential AnimatedTransform::exec(const paladin::RayDifferential &rd) const {
    if (!_actuallyAnimated || rd.time <= _startTime) {
        return _startTransform->exec(rd);
    } else if (rd.time >= _endTime) {
        return _endTransform->exec(rd);
    }
    Transform trsf = interpolate(rd.time);
    return trsf.exec(rd);
}

Transform AnimatedTransform::interpolate(Float time) const {
    if (time <= _startTime || !_actuallyAnimated) {
        return * _startTransform;
    }
    if (time >= _endTime) {
        return * _endTransform;
    }
    // 根据平移旋转缩放三个分量进行插值然后再组合
    Float dt = (time - _startTime) / (_endTime - _startTime);
    Vector3f T = (1 - dt) * _T[0] + dt * _T[1];
    Quaternion R = slerp(dt, _R[0], _R[1]);
    
    Matrix4x4 S;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            S._m[i][j] = lerp(dt, _S[0]._m[i][j], _S[1]._m[i][j]);
    
    return Transform::translate(T) * R.ToTransform() * Transform(S);
}

Bounds3f AnimatedTransform::BoundPointMotion(const Point3f &p) const {
    if (!_actuallyAnimated)
        return Bounds3f(_startTransform->exec(p));
    Bounds3f bounds(_startTransform->exec(p), _endTransform->exec(p));
    Float cosTheta = dot(_R[0], _R[1]);
    Float theta = std::acos(clamp(cosTheta, -1, 1));
    
    // 分xyz三个维度依次求出三个维度极值点
    for (int c = 0; c < 3; ++c) {
        Float zeros[8];
        int nZeros = 0;
        // 找到对应维度的所有极值点
        intervalFindZeros(c1[c].Eval(p), c2[c].Eval(p), c3[c].Eval(p),
                          c4[c].Eval(p), c5[c].Eval(p), theta, Interval(0., 1.),
                          zeros, &nZeros);
        CHECK_LE(nZeros, sizeof(zeros) / sizeof(zeros[0]));
        
        // 扩展根据极值点扩展包围盒
        for (int i = 0; i < nZeros; ++i) {
            Point3f pz = exec(lerp(zeros[i], _startTime, _endTime), p);
            bounds = unionSet(bounds, pz);
        }
    }
    return bounds;
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
