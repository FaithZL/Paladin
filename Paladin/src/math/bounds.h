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

// Bounds Declarations
template <typename T>
class Bounds2 {
public:
    // Bounds2 Public Methods
    Bounds2() {
        T minNum = std::numeric_limits<T>::lowest();
        T maxNum = std::numeric_limits<T>::max();
        pMin = Point2<T>(maxNum, maxNum);
        pMax = Point2<T>(minNum, minNum);
    }
    explicit Bounds2(const Point2<T> &p) : pMin(p), pMax(p) {}
    Bounds2(const Point2<T> &p1, const Point2<T> &p2) {
        pMin = Point2<T>(std::min(p1.x, p2.x), std::min(p1.y, p2.y));
        pMax = Point2<T>(std::max(p1.x, p2.x), std::max(p1.y, p2.y));
    }
    template <typename U>
    explicit operator Bounds2<U>() const {
        return Bounds2<U>((Point2<U>)pMin, (Point2<U>)pMax);
    }
    
    Vector2<T> diagonal() const { return pMax - pMin; }
    T Area() const {
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
    bool operator==(const Bounds2<T> &b) const {
        return b.pMin == pMin && b.pMax == pMax;
    }
    bool operator!=(const Bounds2<T> &b) const {
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
    friend std::ostream &operator<<(std::ostream &os, const Bounds2<T> &b) {
        os << "[ " << b.pMin << " - " << b.pMax << " ]";
        return os;
    }
    
    // Bounds2 Public Data
    Point2<T> pMin, pMax;
};

template <typename T>
class Bounds3 {
public:
    // Bounds3 Public Methods
    Bounds3() {
        T minNum = std::numeric_limits<T>::lowest();
        T maxNum = std::numeric_limits<T>::max();
        pMin = Point3<T>(maxNum, maxNum, maxNum);
        pMax = Point3<T>(minNum, minNum, minNum);
    }
    explicit Bounds3(const Point3<T> &p) : pMin(p), pMax(p) {
        
    }
    Bounds3(const Point3<T> &p1, const Point3<T> &p2)
    : pMin(std::min(p1.x, p2.x), std::min(p1.y, p2.y),
           std::min(p1.z, p2.z)),
    pMax(std::max(p1.x, p2.x), std::max(p1.y, p2.y),
    std::max(p1.z, p2.z)) {
        
    }
    
    const Point3<T> &operator[](int i) const;
    
    Point3<T> &operator[](int i);
    
    bool operator==(const Bounds3<T> &b) const {
        return b.pMin == pMin && b.pMax == pMax;
    }
    
    bool operator!=(const Bounds3<T> &b) const {
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
    explicit operator Bounds3<U>() const {
        return Bounds3<U>((Point3<U>)pMin, (Point3<U>)pMax);
    }
    
    bool intersectP(const Ray &ray, Float *hitt0 = nullptr,
                    Float *hitt1 = nullptr) const;
    
    inline bool intersectP(const Ray &ray, const Vector3f &invDir,
                           const int dirIsNeg[3]) const;
    
    friend std::ostream &operator<<(std::ostream &os, const Bounds3<T> &b) {
        os << "[ " << b.pMin << " - " << b.pMax << " ]";
        return os;
    }
    
    // Bounds3 Public Data
    Point3<T> pMin, pMax;
};

template <typename T>
inline const Point3<T> &Bounds3<T>::operator[](int i) const {
    DCHECK(i == 0 || i == 1);
    return (i == 0) ? pMin : pMax;
}

template <typename T>
inline Point3<T> &Bounds3<T>::operator[](int i) {
    DCHECK(i == 0 || i == 1);
    return (i == 0) ? pMin : pMax;
}

template <typename T>
inline bool Bounds3<T>::intersectP(const Ray &ray, Float *hitt0,
                                   Float *hitt1) const {
    Float t0 = 0, t1 = ray.tMax;
    // bound可以理解三对互相垂直的平行面组成的范围
    for (int i = 0; i < 3; ++i) {
        // 计算ray与每个维度的slab的两个交点t值
        Float invRayDir = 1 / ray.d[i];
        Float tNear = (pMin[i] - ray.o[i]) * invRayDir;
        Float tFar = (pMax[i] - ray.o[i]) * invRayDir;

        if (tNear > tFar) std::swap(tNear, tFar);

        // 用于误差分析
        tFar *= 1 + 2 * paladin::gamma(3);
        t0 = tNear > t0 ? tNear : t0;
        t1 = tFar < t1 ? tFar : t1;
        if (t0 > t1) return false;
    }
    if (hitt0) *hitt0 = t0;
    if (hitt1) *hitt1 = t1;
    return true;
}

typedef Bounds2<Float> Bounds2f;
typedef Bounds2<int> Bounds2i;
typedef Bounds3<Float> Bounds3f;
typedef Bounds3<int> Bounds3i;

template <typename T>
inline bool Bounds3<T>::intersectP(const Ray &ray, const Vector3f &invDir,
                                   const int dirIsNeg[3]) const {
    // 总体思路，先用x方向求出两个交点t值，再加入y方向更新t值，最后加入z方向更新t值
    //dirIsNeg为数组，表示ray方向的三个分量是否为负，dirIsNeg[0]=1表示，x方向为负，以此类推
    // 此方法只需要返回是否相交，不需要求交点，所以效率比上一个求交点的要高
    const Bounds3f &bounds = *this;
    // 以下三个维度的t值均为ray沿着d方向的t值，已经考虑了方向，因此可以直接比较t值之间的大小可以确定是否相交
    // 首先求出xy两个方向维度与slab的四个交点
    Float tMin = (bounds[dirIsNeg[0]].x - ray.o.x) * invDir.x;
    Float tMax = (bounds[1 - dirIsNeg[0]].x - ray.o.x) * invDir.x;
    Float tyMin = (bounds[dirIsNeg[1]].y - ray.o.y) * invDir.y;
    Float tyMax = (bounds[1 - dirIsNeg[1]].y - ray.o.y) * invDir.y;

    // gamma函数用于误差分析
    tMax *= 1 + 2 * paladin::gamma(3);
    tyMax *= 1 + 2 * paladin::gamma(3);
    if (tMin > tyMax || tyMin > tMax) return false;
    
    // 如果xy两个维度的交点在bound以内，则更新两个交点位置，修正两个交点的t值
    if (tyMin > tMin) tMin = tyMin;
    if (tyMax < tMax) tMax = tyMax;

    // 求出z方向的t值
    Float tzMin = (bounds[dirIsNeg[2]].z - ray.o.z) * invDir.z;
    Float tzMax = (bounds[1 - dirIsNeg[2]].z - ray.o.z) * invDir.z;

    tzMax *= 1 + 2 * paladin::gamma(3);
    
    if (tMin > tzMax || tzMin > tMax) return false;
    
    if (tzMin > tMin) tMin = tzMin;
    if (tzMax < tMax) tMax = tzMax;
    return (tMin < ray.tMax) && (tMax > 0);
}



class Bounds2iIterator : public std::forward_iterator_tag {
public:
    Bounds2iIterator(const Bounds2i &b, const Point2i &pt)
    : p(pt), bounds(&b) {}
    Bounds2iIterator operator++() {
        advance();
        return *this;
    }
    Bounds2iIterator operator++(int) {
        Bounds2iIterator old = *this;
        advance();
        return old;
    }
    bool operator==(const Bounds2iIterator &bi) const {
        return p == bi.p && bounds == bi.bounds;
    }
    bool operator!=(const Bounds2iIterator &bi) const {
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
    const Bounds2i *bounds;
};

inline Bounds2iIterator begin(const Bounds2i &b) {
    return Bounds2iIterator(b, b.pMin);
}

inline Bounds2iIterator end(const Bounds2i &b) {
    // Normally, the ending point is at the minimum x value and one past
    // the last valid y value.
    Point2i pEnd(b.pMin.x, b.pMax.y);
    // However, if the bounds are degenerate, override the end point to
    // equal the start point so that any attempt to iterate over the bounds
    // exits out immediately.
    if (b.pMin.x >= b.pMax.x || b.pMin.y >= b.pMax.y)
        pEnd = b.pMin;
    return Bounds2iIterator(b, pEnd);
}

PALADIN_END

#endif /* bound_h */

