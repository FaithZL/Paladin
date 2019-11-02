//
//  sphere.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef sphere_hpp
#define sphere_hpp

#include "core/header.h"
#include "core/shape.hpp"

PALADIN_BEGIN

/**
 * Sphere类，可以定义球体，以及部分球体
 * 可以把球体当做一个180°的圆弧以z轴旋转360°得到的一个回转体
 * 所有球体对象的object坐标系都是以球心为原点
 * 以z坐标的范围跟φ来裁剪球体，
 * 其中
 * -r ≤ z ≤ r
 * 0 < φ ≤ 2π ，φ为转动的角度
 * 形状请自行脑补，完全ojbk
 * 
 * 球的参数方程可以表示为
 * x = rsinθcosφ
 * y = rsinθsinφ
 * z = rcosθ
 * 
 * x^2 + y^2 +z^2 − r^2 =0.
 * 
 * 表面参数uv为
 * φ = u * φmax
 * θ = θmin + v * (θmax - θmin)
 */
class Sphere : public Shape {
    
public:
    Sphere(Transform *objectToWorld,
           Transform *worldToObject,
           bool reverseOrientation,
           Float radius,
           Float zMax,
           Float zMin,
           Float phiMax):
    Shape(objectToWorld, worldToObject, reverseOrientation),
    _radius(radius),
    _zMin(clamp(std::min(zMax, zMin), -radius, radius)),
    _zMax(clamp(std::max(zMax, zMin), -radius, radius)),
    _phiMax(degree2radian(clamp(phiMax, 0, 360))),
    _thetaMin(std::acos(clamp(zMin / radius, -1, 1))),
    _thetaMax(std::acos(clamp(zMax / radius, -1, 1))) {
        init();
    }
    
    virtual void init() {
        _invArea = 1.f / area();
    }
    
    virtual AABB3f objectBound() const {
        // 这里似乎可以优化一下，bound的范围可以再小些，todo
        return AABB3f(Point3f(-_radius, -_radius, _zMin),
                    Point3f(_radius, _radius, _zMax));
    }
    
    /**
     * 基本思路
     * 1.直接求解直线与球体的交点
     * 2.判断交点是否在ray的范围内
     * 3.判断交点是否在sphere有效部分之内
     */
    virtual bool intersect(const Ray &ray, Float *tHit, SurfaceInteraction *isect, bool testAlphaTexture) const;
    
    virtual bool intersectP(const Ray &ray, bool testAlphaTexture) const;
    
    /**
     * 可以把球体当做一个180°的圆弧以z轴旋转360°得到的一个回转体
     * 球体的一部分则可以当做一个圆弧以z轴旋转φ
     * 由高等数学知识可得，回转体的表面积为
     * area = φ * ∫[zMin, zMax]f(z) * sqrt(1 + (f'(z))^2)dz
     * f(z) = sqrt(r^2 - z^2)
     * f'(z) = -(z / sqrt(r^2 - z^2))
     * 积分可得 area = φ * r * (zMax - zMin)
     */
    virtual Float area() const {
        return _phiMax * _radius * (_zMax - _zMin);
    }
    
    virtual Interaction sampleA(const Point2f &u, Float *pdf) const;
    
    virtual Interaction sampleW(const Interaction &ref, const Point2f &u, Float *pdf) const;
    
    virtual Float pdfDir(const Interaction &ref, const Vector3f &wi) const;
    
    virtual Float solidAngle(const Point3f &p, int nSamples = 512) const {
        Point3f pCenter = objectToWorld->exec(Point3f(0, 0, 0));
        Float r2 = _radius * _radius;
        if (distanceSquared(p, pCenter) <= r2) {
            return 4 * Pi;
        }
        Float sinTheta2 = r2 / distanceSquared(p, pCenter);
        Float cosTheta = std::sqrt(std::max((Float)0, 1 - sinTheta2));
        return (2 * Pi * (1 - cosTheta));
    }
    
private:
    
    const Float _radius;
    const Float _zMin;
    const Float _zMax;
    const Float _phiMax;
    const Float _thetaMin;
    const Float _thetaMax;
};

PALADIN_END

#endif /* sphere_hpp */
