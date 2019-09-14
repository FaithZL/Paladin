//
//  bounds.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef bounds_h
#define bounds_h

PALADIN_BEGIN

// AABB Declarations
template <typename T>
class AABB2 {
public:
    // AABB2 Public Methods
    AABB2() {
        T minNum = std::numeric_limits<T>::lowest();
        T maxNum = std::numeric_limits<T>::max();
        pMin = Point2<T>(maxNum, maxNum);
        pMax = Point2<T>(minNum, minNum);
    }
    explicit AABB2(const Point2<T> &p) : pMin(p), pMax(p) {}
    AABB2(const Point2<T> &p1, const Point2<T> &p2) {
        pMin = Point2<T>(std::min(p1.x, p2.x), std::min(p1.y, p2.y));
        pMax = Point2<T>(std::max(p1.x, p2.x), std::max(p1.y, p2.y));
    }
    template <typename U>
    explicit operator AABB2<U>() const {
        return AABB2<U>((Point2<U>)pMin, (Point2<U>)pMax);
    }
    
    Vector2<T> diagonal() const { return pMax - pMin; }
    T area() const {
        Vector2<T> d = pMax - pMin;
        return (d.x * d.y);
    }
    int maximumExtent() const {
        Vector2<T> diag = diagonal();
        if (diag.x > diag.y)
            return 0;
        else
            return 1;
    }
    inline const Point2<T> &operator[](int i) const {
        DCHECK(i == 0 || i == 1);
        return (i == 0) ? pMin : pMax;
    }
    inline Point2<T> &operator[](int i) {
        DCHECK(i == 0 || i == 1);
        return (i == 0) ? pMin : pMax;
    }
    bool operator==(const AABB2<T> &b) const {
        return b.pMin == pMin && b.pMax == pMax;
    }
    bool operator!=(const AABB2<T> &b) const {
        return b.pMin != pMin || b.pMax != pMax;
    }
    Point2<T> lerp(const Point2f &t) const {
        return Point2<T>(paladin::lerp(t.x, pMin.x, pMax.x),
                         paladin::lerp(t.y, pMin.y, pMax.y));
    }
    Vector2<T> offset(const Point2<T> &p) const {
        Vector2<T> o = p - pMin;
        if (pMax.x > pMin.x) o.x /= pMax.x - pMin.x;
        if (pMax.y > pMin.y) o.y /= pMax.y - pMin.y;
        return o;
    }
    void boundingSphere(Point2<T> *c, Float *rad) const {
        *c = (pMin + pMax) / 2;
        *rad = inside(*c, *this) ? distance(*c, pMax) : 0;
    }
    friend std::ostream &operator<<(std::ostream &os, const AABB2<T> &b) {
        os << "[ " << b.pMin << " - " << b.pMax << " ]";
        return os;
    }
    
    // AABB2 Public Data
    Point2<T> pMin, pMax;
};

template <typename T>
class AABB3 {
public:
    // AABB3 Public Methods
    AABB3() {
        T minNum = std::numeric_limits<T>::lowest();
        T maxNum = std::numeric_limits<T>::max();
        pMin = Point3<T>(maxNum, maxNum, maxNum);
        pMax = Point3<T>(minNum, minNum, minNum);
    }
    explicit AABB3(const Point3<T> &p) : pMin(p), pMax(p) {
        
    }
    AABB3(const Point3<T> &p1, const Point3<T> &p2)
    : pMin(std::min(p1.x, p2.x), std::min(p1.y, p2.y),
           std::min(p1.z, p2.z)),
    pMax(std::max(p1.x, p2.x), std::max(p1.y, p2.y),
    std::max(p1.z, p2.z)) {
        
    }
    
    const Point3<T> &operator[](int i) const;
    
    Point3<T> &operator[](int i);
    
    bool operator==(const AABB3<T> &b) const {
        return b.pMin == pMin && b.pMax == pMax;
    }
    
    bool operator!=(const AABB3<T> &b) const {
        return b.pMin != pMin || b.pMax != pMax;
    }
    
    Point3<T> corner(int corner) const {
        DCHECK(corner >= 0 && corner < 8);
        return Point3<T>((*this)[(corner & 1)].x,
                         (*this)[(corner & 2) ? 1 : 0].y,
                         (*this)[(corner & 4) ? 1 : 0].z);
    }
    
    Vector3<T> diagonal() const {
        return pMax - pMin;
    }
    
    T surfaceArea() const {
        Vector3<T> d = diagonal();
        return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
    }
    
    T volume() const {
        Vector3<T> d = diagonal();
        return d.x * d.y * d.z;
    }
    
    int maximumExtent() const {
        Vector3<T> d = diagonal();
        if (d.x > d.y && d.x > d.z)
            return 0;
        else if (d.y > d.z)
            return 1;
        else
            return 2;
    }
    
    Point3<T> lerp(const Point3f &t) const {
        return Point3<T>(paladin::lerp(t.x, pMin.x, pMax.x),
                         paladin::lerp(t.y, pMin.y, pMax.y),
                         paladin::lerp(t.z, pMin.z, pMax.z));
    }
    
    Vector3<T> offset(const Point3<T> &p) const {
        Vector3<T> o = p - pMin;
        if (pMax.x > pMin.x) o.x /= pMax.x - pMin.x;
        if (pMax.y > pMin.y) o.y /= pMax.y - pMin.y;
        if (pMax.z > pMin.z) o.z /= pMax.z - pMin.z;
        return o;
    }
    
    void boundingSphere(Point3<T> *center, Float *radius) const {
        *center = (pMin + pMax) / 2;
        *radius = inside(*center, *this) ? distance(*center, pMax) : 0;
    }
    
    template <typename U>
    explicit operator AABB3<U>() const {
        return AABB3<U>((Point3<U>)pMin, (Point3<U>)pMax);
    }
    
    bool intersectP(const Ray &ray, Float *hitt0 = nullptr,
                    Float *hitt1 = nullptr) const;
    
    inline bool intersectP(const Ray &ray, const Vector3f &invDir,
                           const int dirIsNeg[3]) const;
    
    friend std::ostream &operator<<(std::ostream &os, const AABB3<T> &b) {
        os << "[ " << b.pMin << " - " << b.pMax << " ]";
        return os;
    }
    
    // AABB3 Public Data
    Point3<T> pMin, pMax;
};

template <typename T>
inline const Point3<T> &AABB3<T>::operator[](int i) const {
    DCHECK(i == 0 || i == 1);
    return (i == 0) ? pMin : pMax;
}

template <typename T>
inline Point3<T> &AABB3<T>::operator[](int i) {
    DCHECK(i == 0 || i == 1);
    return (i == 0) ? pMin : pMax;
}

/*
以下是误差分析的说明，假设读这样已经明白ray与bound求交的算法。
计算ray与bound的交点
假设ray与bound某个平面相交的点为(x,y,z)，则t与x满足如下关系
t = (x − Ox)/dx
t = (x ⊖ Ox) ⊗ (1 ⊘ dx) 
∈ ((x − Ox)/dx) * (1 ± ε)^3
∈ ((x − Ox)/dx) * (1 ± γ3)
嗯对，gamma(3)就是这么计算出来的

以上方法求出的t值是有个误差的
如果两个t值的范围不重合，则直接对比大小(当t0 > t1时，返回false)
如果两个t值的范围重合（t1可能大于t0，也可以小于t0）则需要保守估计，返回结果为相交
所以，要想让结果更加保守，就尽可能提高t1，降低t0
    |<------------------t1-------------------->|
                  |<------t0------>|
________________________________________________________

综合以上两种情况
代码可以表示成：t1 * (1 + γ3)，t0 * (1 - γ3)
但在实际编码过程中，可以单方面的增加t0，保持t1不变
如果t0 t1相差较大，如下写法对返回结果无影响
如果t0与t1非常接近
则可以写成：t0 * (1 + 2 * γ3)，t1保持不变，如下代码所示
 */
template <typename T>
inline bool AABB3<T>::intersectP(const Ray &ray, Float *hitt0,
                                   Float *hitt1) const {
    Float t0 = 0, t1 = ray.tMax;
    // bound可以理解三对互相垂直的平行面组成的范围
    for (int i = 0; i < 3; ++i) {
        // 计算ray与每个维度的slab的两个交点t值
        Float invRayDir = 1 / ray.dir[i];
        Float tNear = (pMin[i] - ray.ori[i]) * invRayDir;
        Float tFar = (pMax[i] - ray.ori[i]) * invRayDir;

        if (tNear > tFar) std::swap(tNear, tFar);

        // 用于误差分析
        tFar *= 1 + 2 * paladin::gamma(3);
        t0 = tNear > t0 ? tNear : t0;
        t1 = tFar < t1 ? tFar : t1;
        if (t0 > t1) return false;
    }
    if (hitt0) {
        *hitt0 = t0;
    }
    if (hitt1) {
        *hitt1 = t1;
    }
    return true;
}

typedef AABB2<Float> AABB2f;
typedef AABB2<int> AABB2i;
typedef AABB3<Float> AABB3f;
typedef AABB3<int> AABB3i;

template <typename T>
inline bool AABB3<T>::intersectP(const Ray &ray, const Vector3f &invDir,
                                   const int dirIsNeg[3]) const {
    // 总体思路，先用x方向求出两个交点t值，再加入y方向更新t值，最后加入z方向更新t值
    //dirIsNeg为数组，表示ray方向的三个分量是否为负，dirIsNeg[0]=1表示，x方向为负，以此类推
    // 此方法只需要返回是否相交，不需要求交点，所以效率比上一个求交点的要高
    const AABB3f &bounds = *this;
    // 以下三个维度的t值均为ray沿着d方向的t值，已经考虑了方向，因此可以直接比较t值之间的大小可以确定是否相交
    // 首先求出xy两个方向维度与slab的四个交点
    Float tMin = (bounds[dirIsNeg[0]].x - ray.ori.x) * invDir.x;
    Float tMax = (bounds[1 - dirIsNeg[0]].x - ray.ori.x) * invDir.x;
    Float tyMin = (bounds[dirIsNeg[1]].y - ray.ori.y) * invDir.y;
    Float tyMax = (bounds[1 - dirIsNeg[1]].y - ray.ori.y) * invDir.y;

    // gamma函数用于误差分析
    tMax *= 1 + 2 * paladin::gamma(3);
    tyMax *= 1 + 2 * paladin::gamma(3);
    if (tMin > tyMax || tyMin > tMax) return false;
    
    // 如果xy两个维度的交点在bound以内，则更新两个交点位置，修正两个交点的t值
    if (tyMin > tMin) tMin = tyMin;
    if (tyMax < tMax) tMax = tyMax;

    // 求出z方向的t值
    Float tzMin = (bounds[dirIsNeg[2]].z - ray.ori.z) * invDir.z;
    Float tzMax = (bounds[1 - dirIsNeg[2]].z - ray.ori.z) * invDir.z;

    tzMax *= 1 + 2 * paladin::gamma(3);
    
    if (tMin > tzMax || tzMin > tMax) return false;
    
    if (tzMin > tMin) tMin = tzMin;
    if (tzMax < tMax) tMax = tzMax;
    return (tMin < ray.tMax) && (tMax > 0);
}


// bounds2i的迭代器，用于遍历bounds2区域内的所有离散点
class AABB2iIterator : public std::forward_iterator_tag {
public:
    AABB2iIterator(const AABB2i &b, const Point2i &pt)
    : p(pt), bounds(&b) {}
    AABB2iIterator operator++() {
        advance();
        return *this;
    }
    AABB2iIterator operator++(int) {
        AABB2iIterator old = *this;
        advance();
        return old;
    }
    bool operator==(const AABB2iIterator &bi) const {
        return p == bi.p && bounds == bi.bounds;
    }
    bool operator!=(const AABB2iIterator &bi) const {
        return p != bi.p || bounds != bi.bounds;
    }
    
    Point2i operator*() const { return p; }
    
private:
    void advance() {
        ++p.x;
        if (p.x == bounds->pMax.x) {
            p.x = bounds->pMin.x;
            ++p.y;
        }
    }
    Point2i p;
    const AABB2i *bounds;
};

inline AABB2iIterator begin(const AABB2i &b) {
    return AABB2iIterator(b, b.pMin);
}

inline AABB2iIterator end(const AABB2i &b) {
    // end迭代器返回最后一个点的下一个位置
    // 一般来说是最后一个点在pMax的下一个点
    Point2i pEnd(b.pMin.x, b.pMax.y);
    // 但如果bounds退化为一个点时，begin的位置等于end的位置，遍历bounds的操作将不会进行
    if (b.pMin.x >= b.pMax.x || b.pMin.y >= b.pMax.y)
        pEnd = b.pMin;
    return AABB2iIterator(b, pEnd);
}

PALADIN_END

#endif /* bound_h */

