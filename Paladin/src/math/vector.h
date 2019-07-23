//
//  vector.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef vector_h
#define vector_h

#include "header.h"

PALADIN_BEGIN

// Vector Declarations
template <typename T>
class Vector2 {
public:
    // Vector2 Public Methods
    Vector2() { x = y = 0; }
    Vector2(T xx, T yy) : x(xx), y(yy) { 
        // DCHECK(!HasNaNs()); 
    }
    bool hasNaNs() const { return isNaN(x) || isNaN(y); }
    explicit Vector2(const Point2<T> &p);
    explicit Vector2(const Point3<T> &p);
#ifndef NDEBUG
    // The default versions of these are fine for release builds; for debug
    // we define them so that we can add the Assert checks.
    Vector2(const Vector2<T> &v) {
        // DCHECK(!v.HasNaNs());
        x = v.x;
        y = v.y;
    }
    Vector2<T> &operator=(const Vector2<T> &v) {
        // DCHECK(!v.HasNaNs());
        x = v.x;
        y = v.y;
        return *this;
    }
#endif  // !NDEBUG
    
    Vector2<T> operator+(const Vector2<T> &v) const {
        // DCHECK(!v.HasNaNs());
        return Vector2(x + v.x, y + v.y);
    }
    
    Vector2<T> &operator+=(const Vector2<T> &v) {
        // DCHECK(!v.HasNaNs());
        x += v.x;
        y += v.y;
        return *this;
    }
    Vector2<T> operator-(const Vector2<T> &v) const {
        // DCHECK(!v.HasNaNs());
        return Vector2(x - v.x, y - v.y);
    }
    
    Vector2<T> &operator-=(const Vector2<T> &v) {
        // DCHECK(!v.HasNaNs());
        x -= v.x;
        y -= v.y;
        return *this;
    }
    bool operator==(const Vector2<T> &v) const { return x == v.x && y == v.y; }
    bool operator!=(const Vector2<T> &v) const { return x != v.x || y != v.y; }
    template <typename U>
    Vector2<T> operator*(U f) const {
        return Vector2<T>(f * x, f * y);
    }
    
    template <typename U>
    Vector2<T> &operator*=(U f) {
       DCHECK(!isNaN(f));
        x *= f;
        y *= f;
        return *this;
    }
    template <typename U>
    Vector2<T> operator/(U f) const {
//        CHECK_NE(f, 0);
        Float inv = (Float)1 / f;
        return Vector2<T>(x * inv, y * inv);
    }
    
    template <typename U>
    Vector2<T> &operator/=(U f) {
//        CHECK_NE(f, 0);
        Float inv = (Float)1 / f;
        x *= inv;
        y *= inv;
        return *this;
    }
    Vector2<T> operator-() const { return Vector2<T>(-x, -y); }
    T operator[](int i) const {
//       DCHECK(i >= 0 && i <= 1);
        if (i == 0) return x;
        return y;
    }
    
    T &operator[](int i) {
//       DCHECK(i >= 0 && i <= 1);
        if (i == 0) return x;
        return y;
    }

    Float lengthSquared() const { 
        return x * x + y * y;
    }

    Float length() const { 
        return std::sqrt(lengthSquared()); 
    }

    bool isZero() const {
        return x == 0 && y == 0;
    }
    
    // Vector2 Public Data
    T x;
    T y;
};

template <typename T>
Vector2<T>::Vector2(const Point2<T> &p)
: x(p.x), y(p.y) {
    DCHECK(!hasNaNs());
}

template <typename T>
Vector2<T>::Vector2(const Point3<T> &p)
: x(p.x), y(p.y) {
    DCHECK(!hasNaNs());
}

template <typename T>
inline std::ostream &operator<<(std::ostream &os, const Vector2<T> &v) {
    os << "[ " << v.x << ", " << v.y << " ]";
    return os;
}

template <>
inline std::ostream &operator<<(std::ostream &os, const Vector2<Float> &v) {
    os << StringPrintf("[ %f, %f ]", v.x, v.y);
    return os;
}


template <typename T>
class Vector3 {
public:
    
    Vector3() { x = y = z = 0; }
    
    Vector3(T x, T y, T z) : x(x),y(y),z(z) {
        
    }
    
    T operator[](int i) const {
        // DCHECK(i >= 0 && i <= 2);
        if (i == 0) return x;
        if (i == 1) return y;
        return z;
    }
    T &operator[](int i) {
        // DCHECK(i >= 0 && i <= 2);
        if (i == 0) return x;
        if (i == 1) return y;
        return z;
    }
    
    
    bool hasNaNs() const { return isNaN(x) || isNaN(y) || isNaN(z); }
    
    explicit Vector3(const Point3<T> &p);
#ifndef NDEBUG
    // The default versions of these are fine for release builds; for debug
    // we define them so that we can add the Assert checks.
    Vector3(const Vector3<T> &v) {
        // DCHECK(!v.HasNaNs());
        x = v.x;
        y = v.y;
        z = v.z;
    }
    
    Direction3<T> getDirection3() const {
        Direction3<T> ret(x, y, z);
        return ret;
    }
    
    Vector3<T> &operator=(const Vector3<T> &v) {
        // DCHECK(!v.HasNaNs());
        x = v.x;
        y = v.y;
        z = v.z;
        return *this;
    }
#endif  // !NDEBUG
    Vector3<T> operator+(const Vector3<T> &v) const {
        // DCHECK(!v.HasNaNs());
        return Vector3(x + v.x, y + v.y, z + v.z);
    }
    
    Vector3<T> &operator+=(const Vector3<T> &v) {
        // DCHECK(!v.HasNaNs());
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }
    
    Vector3<T> operator-(const Vector3<T> &v) const {
        // DCHECK(!v.HasNaNs());
        return Vector3(x - v.x, y - v.y, z - v.z);
    }
    
    Vector3<T> &operator-=(const Vector3<T> &v) {
        // DCHECK(!v.HasNaNs());
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }
    
    bool operator==(const Vector3<T> &v) const {
        return x == v.x && y == v.y && z == v.z;
    }
    
    bool operator!=(const Vector3<T> &v) const {
        return x != v.x || y != v.y || z != v.z;
    }
    
    template <typename U>
    Vector3<T> operator*(U s) const {
        return Vector3<T>(s * x, s * y, s * z);
    }
    
    template <typename U>
    Vector3<T> &operator*=(U s) {
        // DCHECK(!isNaN(s));
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }
    
    template <typename U>
    Vector3<T> operator/(U f) const {
        CHECK_NE(f, 0);
        Float inv = (Float)1 / f;
        return Vector3<T>(x * inv, y * inv, z * inv);
    }
    
    template <typename U>
    Vector3<T> &operator/=(U f) {
        CHECK_NE(f, 0);
        Float inv = (Float)1 / f;
        x *= inv;
        y *= inv;
        z *= inv;
        return *this;
    }
    Vector3<T> operator-() const { 
        return Vector3<T>(-x, -y, -z); 
    }

    Float lengthSquared() const { 
        return x * x + y * y + z * z; 
    }

    Float length() const { 
        return std::sqrt(lengthSquared()); 
    }

    bool isZero() const {
        return x == 0 && y == 0 && z == 0;
    }

    explicit Vector3(const Normal3<T> &n);
    
    // Vector3 Public Data
    T x;
    T y;
    T z;
};

template <typename T>
inline Vector3<T>::Vector3(const Point3<T> &p)
: x(p.x), y(p.y), z(p.z) {
    DCHECK(!hasNaNs());
}

template <typename T>
inline std::ostream &operator<<(std::ostream &os, const Vector3<T> &v) {
    os << "[ " << v.x << ", " << v.y << ", " << v.z << " ]";
    return os;
}

template <>
inline std::ostream &operator<<(std::ostream &os, const Vector3<Float> &v) {
    os << StringPrintf("[ %f, %f, %f ]", v.x, v.y, v.z);
    return os;
}



typedef Vector2<Float> Vector2f;
typedef Vector2<int> Vector2i;
typedef Vector3<Float> Vector3f;
typedef Vector3<int> Vector3i;

template <typename T>
class Normal3 {
public:
    // Normal3 Public Methods
    Normal3() { x = y = z = 0; }
    Normal3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) { DCHECK(!hasNaNs()); }
    Normal3<T> operator-() const { return Normal3(-x, -y, -z); }
    Normal3<T> operator+(const Normal3<T> &n) const {
        DCHECK(!n.HasNaNs());
        return Normal3<T>(x + n.x, y + n.y, z + n.z);
    }
    
    Normal3<T> &operator+=(const Normal3<T> &n) {
        DCHECK(!n.HasNaNs());
        x += n.x;
        y += n.y;
        z += n.z;
        return *this;
    }
    Normal3<T> operator-(const Normal3<T> &n) const {
        DCHECK(!n.HasNaNs());
        return Normal3<T>(x - n.x, y - n.y, z - n.z);
    }
    
    Normal3<T> &operator-=(const Normal3<T> &n) {
        DCHECK(!n.HasNaNs());
        x -= n.x;
        y -= n.y;
        z -= n.z;
        return *this;
    }
    bool hasNaNs() const { return isNaN(x) || isNaN(y) || isNaN(z); }
    template <typename U>
    Normal3<T> operator*(U f) const {
        return Normal3<T>(f * x, f * y, f * z);
    }
    
    template <typename U>
    Normal3<T> &operator*=(U f) {
        x *= f;
        y *= f;
        z *= f;
        return *this;
    }
    template <typename U>
    Normal3<T> operator/(U f) const {
        CHECK_NE(f, 0);
        Float inv = (Float)1 / f;
        return Normal3<T>(x * inv, y * inv, z * inv);
    }
    
    template <typename U>
    Normal3<T> &operator/=(U f) {
        CHECK_NE(f, 0);
        Float inv = (Float)1 / f;
        x *= inv;
        y *= inv;
        z *= inv;
        return *this;
    }
    Float lengthSquared() const { return x * x + y * y + z * z; }
    Float length() const { return std::sqrt(lengthSquared()); }
    
#ifndef NDEBUG
    Normal3<T>(const Normal3<T> &n) {
        DCHECK(!n.hasNaNs());
        x = n.x;
        y = n.y;
        z = n.z;
    }
    
    Normal3<T> &operator=(const Normal3<T> &n) {
        DCHECK(!n.hasNaNs());
        x = n.x;
        y = n.y;
        z = n.z;
        return *this;
    }
#endif  // !NDEBUG
    explicit Normal3<T>(const Vector3<T> &v) : x(v.x), y(v.y), z(v.z) {
        DCHECK(!v.HasNaNs());
    }
    bool operator==(const Normal3<T> &n) const {
        return x == n.x && y == n.y && z == n.z;
    }
    bool operator!=(const Normal3<T> &n) const {
        return x != n.x || y != n.y || z != n.z;
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

    bool isZero() const {
        return x == 0 && y == 0 && z == 0;
    }
    
    // Normal3 Public Data
    T x;
    T y;
    T z;
};


template <typename T>
inline std::ostream &operator<<(std::ostream &os, const Normal3<T> &v) {
    os << "[ " << v.x << ", " << v.y << ", " << v.z << " ]";
    return os;
}

template <>
inline std::ostream &operator<<(std::ostream &os, const Normal3<Float> &v) {
    os << StringPrintf("[ %f, %f, %f ]", v.x, v.y, v.z);
    return os;
}

typedef Normal3<Float> Normal3f;

PALADIN_END

#endif /* vector_h */
