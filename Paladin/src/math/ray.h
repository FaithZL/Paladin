//
//  ray.h
//  Paladin
//
//  Created by SATAN_Z on 2019/7/6.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef ray_h
#define ray_h

#include "core/header.h"

PALADIN_BEGIN

/*
ray类由一个点作为起点，一个单位向量作为方向
tMax决定了ray的最远距离
*/
class Ray {
public:
    Ray() : tMax(Infinity), time(0.f), medium(nullptr) {
        
    }
    
    Ray(const Point3f &ori, const Vector3f &dir, Float tMax = Infinity,
        Float time = 0.f, const Medium *medium = nullptr)
    : ori(ori),
    dir(dir),
    tMax(tMax),
    time(time),
    medium(medium) {
        
    }
    // 个人风格，不喜欢重载括号，改用at
    Point3f at(Float t) const {
        return ori + dir * t;
    }
    
    bool hasNaNs() const {
        return (ori.hasNaNs() || dir.hasNaNs() || isNaN(tMax));
    }
    
    friend std::ostream &operator<<(std::ostream &os, const Ray &r) {
        os << "[o=" << r.ori << ", d=" << r.dir << ", tMax=" << r.tMax
        << ", time=" << r.time << "]";
        return os;
    }
    
    // 起点
    Point3f ori;
    // 方向，单位向量
    Vector3f dir;
    // 光线的最远距离
    mutable Float tMax;
    // 发射的时间，用于做motion blur
    Float time;
    // 光线所在的介质(水，空气，玻璃等)
    const Medium *medium;
};

/**
 * 从相机从生成的光线微分，辅助光线的起点与主光线起点相同，
 * 经过高光反射或高光投射之后会渐渐偏离
 */
class RayDifferential : public Ray {
public:
    RayDifferential() {
        hasDifferentials = false;
    }

    RayDifferential(const Point3f &ori, const Vector3f &dir, Float tMax = Infinity,
                    Float time = 0.f, const Medium *medium = nullptr)
    : Ray(ori, dir, tMax, time, medium) {
        hasDifferentials = false;
    }

    RayDifferential(const Ray &ray) : Ray(ray) {
        hasDifferentials = false;
    }

    bool HasNaNs() const {
        return Ray::hasNaNs() ||
                (hasDifferentials &&
                 (rxOrigin.hasNaNs() || ryOrigin.hasNaNs() ||
                  rxDirection.hasNaNs() || ryDirection.hasNaNs()));
    }

    void ScaleDifferentials(Float s) {
        rxOrigin = ori + (rxOrigin - ori) * s;
        ryOrigin = ori + (ryOrigin - ori) * s;
        rxDirection = dir + (rxDirection - dir) * s;
        ryDirection = dir + (ryDirection - dir) * s;
    }

    friend std::ostream &operator<<(std::ostream &os, const RayDifferential &r) {
        os << "[ " << (Ray &)r << " has differentials: " <<
        (r.hasDifferentials ? "true" : "false") << ", xo = " << r.rxOrigin <<
        ", xd = " << r.rxDirection << ", yo = " << r.ryOrigin << ", yd = " <<
        r.ryDirection;
        return os;
    }

    bool hasDifferentials;
    // x,y方向微分光线的起点
    Point3f rxOrigin, ryOrigin;
    // x,y方向微分光线的方向
    Vector3f rxDirection, ryDirection;
};

/**
 * 由于计算出的交点可能会有误差，如果直接把pos作为光线的起点，可能取到的是shape内部的点
 * 如果从内部的点发出光线，则可能产生自相交，为了避免这种情况，通常会对pos做一定的偏移
 * 基本思路：
 * 以计算出的交点p为中心，误差偏移量为包围盒的一个顶点，组成一个最小包围盒b
 * 过p点做垂直于法线的平面s，将s沿着法线方向推到于b相交的最远位置，此时s与法线相交的点p'作为光线起点
 */
static Point3f offsetRayOrigin(const Point3f &p, const Vector3f &pError,
                               const Normal3f &n, const Vector3f &w) {
    // 包围盒b的八个顶点为 (±δx , ±δy , ±δz)
    // 假设平面s的函数为 ax+by+cz = d，法向量为(a,b,c)
    // 将法线的绝对值带入方程，求得d的最大值(注意此处把pError当成一个点带入函数)
    // 注意：这里的法向量是单位向量
    Float d = dot(abs(n), pError);
#ifdef FLOAT_AS_DOUBLE
    // 暂时不理解pbrt为何要这样写
    d *= 1024.;
#endif
    Vector3f offset = d * Vector3f(n);

    if (dot(w, n) < 0) {
        // 判断发射方向是向内还是向外
        offset = -offset;
    } 

    Point3f po = p + offset;
    // 计算更加保守的值
    for (int i = 0; i < 3; ++i) {
        if (offset[i] > 0) {
            po[i] = nextFloatUp(po[i]);
        } else if (offset[i] < 0) {
            po[i] = nextFloatDown(po[i]);
        }
    }
    return po;
}

PALADIN_END

#endif /* ray_h */
