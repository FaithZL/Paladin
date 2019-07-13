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
    
    static void Decompose(const Matrix4x4 &m, Vector3f *T, Quaternion *R,
                          Matrix4x4 *S);
    
    void Interpolate(Float time, Transform *t) const;
    
    Ray operator()(const Ray &r) const;
    
    RayDifferential operator()(const RayDifferential &r) const;
    
    Point3f operator()(Float time, const Point3f &p) const;
    
    Vector3f operator()(Float time, const Vector3f &v) const;
    
    bool HasScale() const {
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
