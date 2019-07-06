//
//  point.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef point_hpp
#define point_hpp

#include "header.hpp"


PALADIN_BEGIN

//template <typename T>
//inline bool isNaN(const T x) {
//    return std::isnan(x);
//}
//
//template <>
//inline bool isNaN(const int x) {
//    return false;
//}

template<typename T>
class Point3;

template <typename T>
class Point2 {
public:
    // Point2 Public Methods
    explicit Point2(const Point3<T> &p) : x(p.x), y(p.y) { DCHECK(!hasNaNs()); }
    Point2() { x = y = 0; }
    Point2(T xx, T yy) : x(xx), y(yy) { DCHECK(!hasNaNs()); }
    
    template <typename U>
    explicit Point2(const Point2<U> &p) {
        x = (T)p.x;
        y = (T)p.y;
        DCHECK(!hasNaNs());
    }
    
    template <typename U>
    explicit Point2(const Vector2<U> &p) {
        x = (T)p.x;
        y = (T)p.y;
        DCHECK(!hasNaNs());
    }
    
    template <typename U>
    explicit operator Vector2<U>() const {
        return Vector2<U>(x, y);
    }
    
#ifndef NDEBUG
    Point2(const Point2<T> &p) {
        DCHECK(!p.hasNaNs());
        x = p.x;
        y = p.y;
    }
    
    Point2<T> &operator=(const Point2<T> &p) {
        DCHECK(!p.hasNaNs());
        x = p.x;
        y = p.y;
        return *this;
    }
#endif  // !NDEBUG
    Point2<T> operator+(const Vector2<T> &v) const {
        DCHECK(!v.hasNaNs());
        return Point2<T>(x + v.x, y + v.y);
    }
    
    Point2<T> &operator+=(const Vector2<T> &v) {
        DCHECK(!v.hasNaNs());
        x += v.x;
        y += v.y;
        return *this;
    }
    Vector2<T> operator-(const Point2<T> &p) const {
        DCHECK(!p.hasNaNs());
        return Vector2<T>(x - p.x, y - p.y);
    }
    
    Point2<T> operator-(const Vector2<T> &v) const {
        DCHECK(!v.hasNaNs());
        return Point2<T>(x - v.x, y - v.y);
    }
    Point2<T> operator-() const { return Point2<T>(-x, -y); }
    Point2<T> &operator-=(const Vector2<T> &v) {
        DCHECK(!v.hasNaNs());
        x -= v.x;
        y -= v.y;
        return *this;
    }
    Point2<T> &operator+=(const Point2<T> &p) {
        DCHECK(!p.hasNaNs());
        x += p.x;
        y += p.y;
        return *this;
    }
    Point2<T> operator+(const Point2<T> &p) const {
        DCHECK(!p.hasNaNs());
        return Point2<T>(x + p.x, y + p.y);
    }
    template <typename U>
    Point2<T> operator*(U f) const {
        return Point2<T>(f * x, f * y);
    }
    template <typename U>
    Point2<T> &operator*=(U f) {
        x *= f;
        y *= f;
        return *this;
    }
    template <typename U>
    Point2<T> operator/(U f) const {
        CHECK_NE(f, 0);
        Float inv = (Float)1 / f;
        return Point2<T>(inv * x, inv * y);
    }
    template <typename U>
    Point2<T> &operator/=(U f) {
        CHECK_NE(f, 0);
        Float inv = (Float)1 / f;
        x *= inv;
        y *= inv;
        return *this;
    }
    T operator[](int i) const {
        DCHECK(i >= 0 && i <= 1);
        if (i == 0) return x;
        return y;
    }
    
    T &operator[](int i) {
        DCHECK(i >= 0 && i <= 1);
        if (i == 0) return x;
        return y;
    }
    bool operator==(const Point2<T> &p) const { return x == p.x && y == p.y; }
    bool operator!=(const Point2<T> &p) const { return x != p.x || y != p.y; }
    bool hasNaNs() const { return isNaN(x) || isNaN(y); }
    
    // Point2 Public Data
    T x, y;
};

template <typename T>
inline std::ostream &operator<<(std::ostream &os, const Point2<T> &v) {
    os << "[ " << v.x << ", " << v.y << " ]";
    return os;
}

template <>
inline std::ostream &operator<<(std::ostream &os, const Point2<Float> &v) {
    os << StringPrintf("[ %f, %f ]", v.x, v.y);
    return os;
}

template <typename T>
class Point3 {
public:
    // Point3 Public Methods
    Point3() { x = y = z = 0; }
    Point3(T x, T y, T z) : x(x), y(y), z(z) { DCHECK(!hasNaNs()); }
    template <typename U>
    explicit Point3(const Point3<U> &p)
    : x((T)p.x), y((T)p.y), z((T)p.z) {
        DCHECK(!hasNaNs());
    }
    template <typename U>
    explicit operator Vector3<U>() const {
        return Vector3<U>(x, y, z);
    }
#ifndef NDEBUG
    Point3(const Point3<T> &p) {
        DCHECK(!p.hasNaNs());
        x = p.x;
        y = p.y;
        z = p.z;
    }
    
    Point3<T> &operator=(const Point3<T> &p) {
        DCHECK(!p.hasNaNs());
        x = p.x;
        y = p.y;
        z = p.z;
        return *this;
    }
#endif  // !NDEBUG
    Point3<T> operator+(const Vector3<T> &v) const {
        DCHECK(!v.hasNaNs());
        return Point3<T>(x + v.x, y + v.y, z + v.z);
    }
    Point3<T> &operator+=(const Vector3<T> &v) {
        DCHECK(!v.hasNaNs());
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }
    Vector3<T> operator-(const Point3<T> &p) const {
        DCHECK(!p.hasNaNs());
        return Vector3<T>(x - p.x, y - p.y, z - p.z);
    }
    Point3<T> operator-(const Vector3<T> &v) const {
        DCHECK(!v.hasNaNs());
        return Point3<T>(x - v.x, y - v.y, z - v.z);
    }
    Point3<T> &operator-=(const Vector3<T> &v) {
        DCHECK(!v.hasNaNs());
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }
    Point3<T> &operator+=(const Point3<T> &p) {
        DCHECK(!p.hasNaNs());
        x += p.x;
        y += p.y;
        z += p.z;
        return *this;
    }
    Point3<T> operator+(const Point3<T> &p) const {
        DCHECK(!p.hasNaNs());
        return Point3<T>(x + p.x, y + p.y, z + p.z);
    }
    template <typename U>
    Point3<T> operator*(U f) const {
        return Point3<T>(f * x, f * y, f * z);
    }
    template <typename U>
    Point3<T> &operator*=(U f) {
        x *= f;
        y *= f;
        z *= f;
        return *this;
    }
    template <typename U>
    Point3<T> operator/(U f) const {
        CHECK_NE(f, 0);
        Float inv = (Float)1 / f;
        return Point3<T>(inv * x, inv * y, inv * z);
    }
    template <typename U>
    Point3<T> &operator/=(U f) {
        CHECK_NE(f, 0);
        Float inv = (Float)1 / f;
        x *= inv;
        y *= inv;
        z *= inv;
        return *this;
    }
    T operator[](int i) const {
        DCHECK(i >= 0 && i <= 2);
        if (i == 0) return x;
        if (i == 1) return y;
        return z;
    }
    
    T &operator[](int i) {
        DCHECK(i >= 0 && i <= 2);
        if (i == 0) return x;
        if (i == 1) return y;
        return z;
    }
    bool operator==(const Point3<T> &p) const {
        return x == p.x && y == p.y && z == p.z;
    }
    bool operator!=(const Point3<T> &p) const {
        return x != p.x || y != p.y || z != p.z;
    }
    bool hasNaNs() const { return isNaN(x) || isNaN(y) || isNaN(z); }
    Point3<T> operator-() const { return Point3<T>(-x, -y, -z); }
    
    // Point3 Public Data
    T x, y, z;
};

template <typename T>
inline std::ostream &operator<<(std::ostream &os, const Point3<T> &v) {
    os << "[ " << v.x << ", " << v.y << ", " << v.z << " ]";
    return os;
}

template <>
inline std::ostream &operator<<(std::ostream &os, const Point3<Float> &v) {
    os << StringPrintf("[ %f, %f, %f ]", v.x, v.y, v.z);
    return os;
}

typedef Point2<Float> Point2f;
typedef Point2<int> Point2i;
typedef Point3<Float> Point3f;
typedef Point3<int> Point3i;

PALADIN_END

#endif /* point_h */




