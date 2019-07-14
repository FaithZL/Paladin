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

/*
两个变换之间的过度，用于实现动态模糊
*/
class AnimatedTransform {
    
    AnimatedTransform(const Transform *startTransform, Float startTime,
                      const Transform *endTransform, Float endTime);
   /*
    基本思路
    1.直接提取出平移变换
    2.利用旋转矩阵为正交矩阵的性质(正交矩阵的逆与转置相等)对剔除了平移变换的矩阵进行极分解
    极分解的思路如下:
    M = TRS
    Mi+1 = 1/2(Mi + inverse(transpose(Mi)))
    重复上诉操作直到Mi收敛到一个固定的值，这个值就是旋转矩阵R
    M = RS
    S = inverse(R) * M
    */
    static void decompose(const Matrix4x4 &m, Vector3f *T, Quaternion *R,
                          Matrix4x4 *S);
    
    void interpolate(Float time, Transform *t) const;
    
    Ray exec(const Ray &r) const;
    
    RayDifferential exec(const RayDifferential &r) const;
    
    Point3f exec(Float time, const Point3f &p) const;
    
    Vector3f exec(Float time, const Vector3f &v) const;
    
    bool hasScale() const {
        return _startTransform->hasScale() || _endTransform->hasScale();
    }
    
    Bounds3f MotionBounds(const Bounds3f &b) const;
    
    Bounds3f BoundPointMotion(const Point3f &p) const;
public:
    
private:
    
    const Transform * _startTransform;
    const Transform * _endTransform;
    const Float _startTime;
    const Float _endTime;
    const bool _actuallyAnimated;
    
    bool hasRotation;
    Vector3f T[2];
    Quaternion R[2];
    Matrix4x4 S[2];
};

PALADIN_END

#endif /* animatedtransform_hpp */
