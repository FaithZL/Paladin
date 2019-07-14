//
//  animatedtransform.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/7/13.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef animatedtransform_hpp
#define animatedtransform_hpp

#include "transform.hpp"
#include "quaternion.hpp"
PALADIN_BEGIN

// Interval Definitions
struct Interval {
    // Interval Public Methods
    Interval(Float v) : low(v), high(v) {}
    Interval(Float v0, Float v1)
    : low(std::min(v0, v1)), high(std::max(v0, v1)) {}
    Interval operator + (const Interval &i) const {
        return Interval(low + i.low, high + i.high);
    }
    Interval operator - (const Interval &i) const {
        return Interval(low - i.high, high - i.low);
    }
    Interval operator * (const Interval &i) const {
        return Interval(std::min(std::min(low * i.low, high * i.low),
                                 std::min(low * i.high, high * i.high)),
                        std::max(std::max(low * i.low, high * i.low),
                                 std::max(low * i.high, high * i.high)));
    }
    Float low, high;
};

inline Interval sin(const Interval &i) {
    CHECK_GE(i.low, 0);
    CHECK_LE(i.high, 2.0001 * Pi);
    Float sinLow = std::sin(i.low), sinHigh = std::sin(i.high);
    if (sinLow > sinHigh) std::swap(sinLow, sinHigh);
    if (i.low < Pi / 2 && i.high > Pi / 2) sinHigh = 1.;
    if (i.low < (3.f / 2.f) * Pi && i.high > (3.f / 2.f) * Pi) sinLow = -1.;
    return Interval(sinLow, sinHigh);
}

inline Interval cos(const Interval &i) {
    CHECK_GE(i.low, 0);
    CHECK_LE(i.high, 2.0001 * Pi);
    Float cosLow = std::cos(i.low), cosHigh = std::cos(i.high);
    if (cosLow > cosHigh) std::swap(cosLow, cosHigh);
    if (i.low < Pi && i.high > Pi) cosLow = -1.;
    return Interval(cosLow, cosHigh);
}


/*
 牛顿迭代法求出零点
 牛顿迭代法公式为 x[i+1] = xi + f(xi) / f'(xi)
 牛顿迭代法思路f(x) = 0，x=r为精确解，x0作为近似解
 过点(x0, f(x0))做切线L，L的方程为 y = f(x0) + f'(x0) * (x - x0)
 L与x轴的交点为 x1 = x0 + f(x0) / f'(x0)，反复迭代求出近似解
 
 设 p' = f(t)
 t[i + 1] = ti + f'(t)/f''(t)
 */
void intervalFindZeros(Float c1, Float c2, Float c3, Float c4, Float c5,
                       Float theta, Interval tInterval, Float *zeros,
                       int *zeroCount, int depth = 8) {
    // Evaluate motion derivative in interval form, return if no zeros
    Interval range = Interval(c1) +
    (Interval(c2) + Interval(c3) * tInterval) *
    cos(Interval(2 * theta) * tInterval) +
    (Interval(c4) + Interval(c5) * tInterval) *
    sin(Interval(2 * theta) * tInterval);
    if (range.low > 0. || range.high < 0. || range.low == range.high) return;
    if (depth > 0) {
        // Split _tInterval_ and check both resulting intervals
        Float mid = (tInterval.low + tInterval.high) * 0.5f;
        intervalFindZeros(c1, c2, c3, c4, c5, theta,
                          Interval(tInterval.low, mid), zeros, zeroCount,
                          depth - 1);
        intervalFindZeros(c1, c2, c3, c4, c5, theta,
                          Interval(mid, tInterval.high), zeros, zeroCount,
                          depth - 1);
    } else {
        // Use Newton's method to refine zero
        Float tNewton = (tInterval.low + tInterval.high) * 0.5f;
        for (int i = 0; i < 4; ++i) {
            Float fNewton =
            c1 + (c2 + c3 * tNewton) * std::cos(2.f * theta * tNewton) +
            (c4 + c5 * tNewton) * std::sin(2.f * theta * tNewton);
            Float fPrimeNewton = (c3 + 2 * (c4 + c5 * tNewton) * theta) *
            std::cos(2.f * tNewton * theta) +
            (c5 - 2 * (c2 + c3 * tNewton) * theta) *
            std::sin(2.f * tNewton * theta);
            if (fNewton == 0 || fPrimeNewton == 0) break;
            tNewton = tNewton - fNewton / fPrimeNewton;
        }
        if (tNewton >= tInterval.low - 1e-3f &&
            tNewton < tInterval.high + 1e-3f) {
            zeros[*zeroCount] = tNewton;
            (*zeroCount)++;
        }
    }
}

/*
两个变换之间的过度，用于实现动态模糊
进行变换的顺序为缩放旋转平移 M = TRS
*/
class AnimatedTransform {

public:
    /*
     求一个点p在经过起始到结束的变换过程中p'的轨迹所构成的包围盒
     点集p' = f(M1, M2, t, p) = T(t)R(t)S(t)p
     如果变换中不包含旋转则p' = T(t)S(t)p ,其中p'的x分量为
     p'x = [(1 - t) * S00 + t * S'00] * px + (1 - t) * d03 + d'03
     该方程为t的线性方程，同理可得y,z分量也是t的线性方程。所以极值点为曲线端点
     如果包含旋转变换。
     p' = T(t)R(t)S(t)p

     要求出p'的轨迹所确定的包围盒，就要求出xyz三个分量的极值，再由这几个极值点通过并集求出包围盒
     求极值点思路很简单，就是利用导函数，直接对函数p'求导会得到一个非常非常复杂的表达式，化简之后如下
     derivative = dp'/dt = c1 + (c2 + c3 * t) * cos(2θt) + (c4 + c5 * t) * sin(2θt)
     ci为三维向量，由两个矩阵以及原始点p决定。
     接下来先要求出以上5个ci，然后求出导函数为0的点。
     由于ci由两个矩阵以及原始点p决定，则c的任意一个分量可以表示为点p各个分量的线性组合
     C = kc + kx * p.x + ky * p.y + kz * p.z;
     系数ci的求法真特么太复杂了，直接照搬pbrt代码😂
     DerivativeTerm为导数项ci

     求导数为零的点还是不复杂，用牛顿迭代法
     */    
    AnimatedTransform(const Transform *startTransform, Float startTime,
                      const Transform *endTransform, Float endTime);
    
   /*
    基本思路
    M = TRS
    1.直接提取出平移变换
    2.利用旋转矩阵为正交矩阵的性质(正交矩阵的逆与转置相等)对剔除了平移变换的矩阵进行极分解
    极分解的思路如下:
    M = RS
    Mi+1 = 1/2(Mi + inverse(transpose(Mi)))
    重复上诉操作直到Mi收敛到一个固定的值，这个值就是旋转矩阵R
    S = inverse(R) * M
    */
    static void decompose(const Matrix4x4 &m, Vector3f *T, Quaternion *R,
                          Matrix4x4 *S);
    
    void interpolate(Float time, Transform *t) const;
    
    Transform interpolate(Float time) const;
    
    Ray exec(const Ray &r) const;
    
    RayDifferential exec(const RayDifferential &r) const;
    
    Point3f exec(Float time, const Point3f &p) const {
        // todo
        return p;
    }
    
    Vector3f exec(Float time, const Vector3f &v) const;
    
    bool hasScale() const {
        return _startTransform->hasScale() || _endTransform->hasScale();
    }
    
    /*
     获取一个包围盒对象，返回运动过程中包围盒扫过的范围的包围盒
     */
    Bounds3f MotionBounds(const Bounds3f &b) const;
    
    /*
     获取点对象，返回运动过程中点扫过的范围的包围盒
     */
    Bounds3f BoundPointMotion(const Point3f &p) const;

private:
    
    const Transform * _startTransform;
    const Transform * _endTransform;
    const Float _startTime;
    const Float _endTime;
    const bool _actuallyAnimated;
    
    bool _hasRotation;
    Vector3f _T[2];
    Quaternion _R[2];
    // todo 觉得缩放分量也可以用向量表示而不需要矩阵，暂时还不知道pbrt为何这样用
    Matrix4x4 _S[2];
    
    struct DerivativeTerm {
        DerivativeTerm() {}
        DerivativeTerm(Float c, Float x, Float y, Float z)
        : kc(c), kx(x), ky(y), kz(z) {}
        Float kc, kx, ky, kz;
        Float Eval(const Point3f &p) const {
            return kc + kx * p.x + ky * p.y + kz * p.z;
        }
    };
    DerivativeTerm c1[3], c2[3], c3[3], c4[3], c5[3];
};

PALADIN_END

#endif /* animatedtransform_hpp */
