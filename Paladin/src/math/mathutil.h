//
//  mathutil.h
//  Paladin
//
//  Created by SATAN_Z on 2019/7/3.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef mathutil_h
#define mathutil_h

PALADIN_BEGIN

template <typename T>
inline bool isNaN(const T x) {
    return std::isnan(x);
}

template <>
inline bool isNaN(const int x) {
    return false;
}

template <typename T, typename U>
inline Vector3<T> operator*(U s, const Vector3<T> &v) {
    return v * s;
}
template <typename T>
Vector3<T> abs(const Vector3<T> &v) {
    return Vector3<T>(std::abs(v.x), std::abs(v.y), std::abs(v.z));
}

template <typename T>
inline T dot(const Vector3<T> &v1, const Vector3<T> &v2) {
    DCHECK(!v1.hasNaNs() && !v2.hasNaNs());
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

template <typename T>
inline T absDot(const Vector3<T> &v1, const Vector3<T> &v2) {
    DCHECK(!v1.hasNaNs() && !v2.hasNaNs());
    return std::abs(dot(v1, v2));
}

template <typename T>
inline Vector3<T> cross(const Vector3<T> &v1, const Vector3<T> &v2) {
    DCHECK(!v1.hasNaNs() && !v2.hasNaNs());
    double v1x = v1.x, v1y = v1.y, v1z = v1.z;
    double v2x = v2.x, v2y = v2.y, v2z = v2.z;
    return Vector3<T>((v1y * v2z) - (v1z * v2y), (v1z * v2x) - (v1x * v2z),
                      (v1x * v2y) - (v1y * v2x));
}

template <typename T>
inline Vector3<T> cross(const Vector3<T> &v1, const Normal3<T> &v2) {
    DCHECK(!v1.hasNaNs() && !v2.hasNaNs());
    double v1x = v1.x, v1y = v1.y, v1z = v1.z;
    double v2x = v2.x, v2y = v2.y, v2z = v2.z;
    return Vector3<T>((v1y * v2z) - (v1z * v2y), (v1z * v2x) - (v1x * v2z),
                      (v1x * v2y) - (v1y * v2x));
}

template <typename T>
inline Vector3<T> cross(const Normal3<T> &v1, const Vector3<T> &v2) {
    DCHECK(!v1.hasNaNs() && !v2.hasNaNs());
    double v1x = v1.x, v1y = v1.y, v1z = v1.z;
    double v2x = v2.x, v2y = v2.y, v2z = v2.z;
    return Vector3<T>((v1y * v2z) - (v1z * v2y), (v1z * v2x) - (v1x * v2z),
                      (v1x * v2y) - (v1y * v2x));
}

template <typename T>
inline Vector3<T> normalize(const Vector3<T> &v) {
    return v / v.length();
}

template <typename T>
T minComponent(const Vector3<T> &v) {
    return std::min(v.x, std::min(v.y, v.z));
}

template <typename T>
T maxComponent(const Vector3<T> &v) {
    return std::max(v.x, std::max(v.y, v.z));
}

template <typename T>
int maxDimension(const Vector3<T> &v) {
    return (v.x > v.y) ? ((v.x > v.z) ? 0 : 2) : ((v.y > v.z) ? 1 : 2);
}

template <typename T>
Vector3<T> min(const Vector3<T> &p1, const Vector3<T> &p2) {
    return Vector3<T>(std::min(p1.x, p2.x), std::min(p1.y, p2.y),
                      std::min(p1.z, p2.z));
}

template <typename T>
Vector3<T> max(const Vector3<T> &p1, const Vector3<T> &p2) {
    return Vector3<T>(std::max(p1.x, p2.x), std::max(p1.y, p2.y),
                      std::max(p1.z, p2.z));
}

template <typename T>
Vector3<T> permute(const Vector3<T> &v, int x, int y, int z) {
    return Vector3<T>(v[x], v[y], v[z]);
}

template <typename T>
inline void coordinateSystem(const Vector3<T> &v1, Vector3<T> *v2,
                             Vector3<T> *v3) {
    if (std::abs(v1.x) > std::abs(v1.y))
        *v2 = Vector3<T>(-v1.z, 0, v1.x) / std::sqrt(v1.x * v1.x + v1.z * v1.z);
    else
        *v2 = Vector3<T>(0, v1.z, -v1.y) / std::sqrt(v1.y * v1.y + v1.z * v1.z);
    *v3 = Cross(v1, *v2);
}

template <typename T, typename U>
inline Vector2<T> operator*(U f, const Vector2<T> &v) {
    return v * f;
}
template <typename T>
inline Float dot(const Vector2<T> &v1, const Vector2<T> &v2) {
    DCHECK(!v1.hasNaNs() && !v2.hasNaNs());
    return v1.x * v2.x + v1.y * v2.y;
}

template <typename T>
inline Float absDot(const Vector2<T> &v1, const Vector2<T> &v2) {
    DCHECK(!v1.hasNaNs() && !v2.hasNaNs());
    return std::abs(dot(v1, v2));
}

template <typename T>
inline Vector2<T> normalize(const Vector2<T> &v) {
    return v / v.length();
}
template <typename T>
Vector2<T> abs(const Vector2<T> &v) {
    return Vector2<T>(std::abs(v.x), std::abs(v.y));
}

template <typename T>
inline Float distance(const Point3<T> &p1, const Point3<T> &p2) {
    return (p1 - p2).length();
}

template <typename T>
inline Float distanceSquared(const Point3<T> &p1, const Point3<T> &p2) {
    return (p1 - p2).lengthSquared();
}

template <typename T, typename U>
inline Point3<T> operator*(U f, const Point3<T> &p) {
    DCHECK(!p.hasNaNs());
    return p * f;
}

template <typename T>
Point3<T> lerp(Float t, const Point3<T> &p0, const Point3<T> &p1) {
    return (1 - t) * p0 + t * p1;
}

template <typename T>
Point3<T> min(const Point3<T> &p1, const Point3<T> &p2) {
    return Point3<T>(std::min(p1.x, p2.x), std::min(p1.y, p2.y),
                     std::min(p1.z, p2.z));
}

template <typename T>
Point3<T> max(const Point3<T> &p1, const Point3<T> &p2) {
    return Point3<T>(std::max(p1.x, p2.x), std::max(p1.y, p2.y),
                     std::max(p1.z, p2.z));
}

template <typename T>
Point3<T> floor(const Point3<T> &p) {
    return Point3<T>(std::floor(p.x), std::floor(p.y), std::floor(p.z));
}

template <typename T>
Point3<T> ceil(const Point3<T> &p) {
    return Point3<T>(std::ceil(p.x), std::ceil(p.y), std::ceil(p.z));
}

template <typename T>
Point3<T> abs(const Point3<T> &p) {
    return Point3<T>(std::abs(p.x), std::abs(p.y), std::abs(p.z));
}

template <typename T>
inline Float distance(const Point2<T> &p1, const Point2<T> &p2) {
    return (p1 - p2).length();
}

template <typename T>
inline Float distanceSquared(const Point2<T> &p1, const Point2<T> &p2) {
    return (p1 - p2).lengthSquared();
}

template <typename T, typename U>
inline Point2<T> operator*(U f, const Point2<T> &p) {
    DCHECK(!p.hasNaNs());
    return p * f;
}

template <typename T>
Point2<T> floor(const Point2<T> &p) {
    return Point2<T>(std::floor(p.x), std::floor(p.y));
}

template <typename T>
Point2<T> ceil(const Point2<T> &p) {
    return Point2<T>(std::ceil(p.x), std::ceil(p.y));
}

template <typename T>
Point2<T> lerp(Float t, const Point2<T> &v0, const Point2<T> &v1) {
    return (1 - t) * v0 + t * v1;
}

template <typename T>
Point2<T> min(const Point2<T> &pa, const Point2<T> &pb) {
    return Point2<T>(std::min(pa.x, pb.x), std::min(pa.y, pb.y));
}

template <typename T>
Point2<T> max(const Point2<T> &pa, const Point2<T> &pb) {
    return Point2<T>(std::max(pa.x, pb.x), std::max(pa.y, pb.y));
}

template <typename T>
Point3<T> permute(const Point3<T> &p, int x, int y, int z) {
    return Point3<T>(p[x], p[y], p[z]);
}

template <typename T, typename U>
inline Normal3<T> operator*(U f, const Normal3<T> &n) {
    return Normal3<T>(f * n.x, f * n.y, f * n.z);
}

template <typename T>
inline Normal3<T> normalize(const Normal3<T> &n) {
    return n / n.length();
}

template <typename T>
inline T dot(const Normal3<T> &n1, const Vector3<T> &v2) {
    DCHECK(!n1.hasNaNs() && !v2.hasNaNs());
    return n1.x * v2.x + n1.y * v2.y + n1.z * v2.z;
}

template <typename T>
inline T dot(const Vector3<T> &v1, const Normal3<T> &n2) {
    DCHECK(!v1.hasNaNs() && !n2.hasNaNs());
    return v1.x * n2.x + v1.y * n2.y + v1.z * n2.z;
}

template <typename T>
inline T dot(const Normal3<T> &n1, const Normal3<T> &n2) {
    DCHECK(!n1.hasNaNs() && !n2.hasNaNs());
    return n1.x * n2.x + n1.y * n2.y + n1.z * n2.z;
}

template <typename T>
inline T absDot(const Normal3<T> &n1, const Vector3<T> &v2) {
    DCHECK(!n1.hasNaNs() && !v2.hasNaNs());
    return std::abs(n1.x * v2.x + n1.y * v2.y + n1.z * v2.z);
}

template <typename T>
inline T absDot(const Vector3<T> &v1, const Normal3<T> &n2) {
    DCHECK(!v1.hasNaNs() && !n2.hasNaNs());
    return std::abs(v1.x * n2.x + v1.y * n2.y + v1.z * n2.z);
}

template <typename T>
inline T absDot(const Normal3<T> &n1, const Normal3<T> &n2) {
    DCHECK(!n1.hasNaNs() && !n2.hasNaNs());
    return std::abs(n1.x * n2.x + n1.y * n2.y + n1.z * n2.z);
}

template <typename T>
inline Normal3<T> faceforward(const Normal3<T> &n, const Vector3<T> &v) {
    return (dot(n, v) < 0.f) ? -n : n;
}

template <typename T>
inline Normal3<T> faceforward(const Normal3<T> &n, const Normal3<T> &n2) {
    return (dot(n, n2) < 0.f) ? -n : n;
}

template <typename T>
inline Vector3<T> faceforward(const Vector3<T> &v, const Vector3<T> &v2) {
    return (dot(v, v2) < 0.f) ? -v : v;
}

template <typename T>
inline Vector3<T> faceforward(const Vector3<T> &v, const Normal3<T> &n2) {
    return (dot(v, n2) < 0.f) ? -v : v;
}

template <typename T>
Normal3<T> abs(const Normal3<T> &v) {
    return Normal3<T>(std::abs(v.x), std::abs(v.y), std::abs(v.z));
}


template <typename T>
Bounds3<T> unionSet(const Bounds3<T> &b, const Point3<T> &p) {
    Bounds3<T> ret;
    ret.pMin = min(b.pMin, p);
    ret.pMax = max(b.pMax, p);
    return ret;
}

template <typename T>
Bounds3<T> unionSet(const Bounds3<T> &b1, const Bounds3<T> &b2) {
    Bounds3<T> ret;
    ret.pMin = min(b1.pMin, b2.pMin);
    ret.pMax = max(b1.pMax, b2.pMax);
    return ret;
}

template <typename T>
Bounds3<T> intersect(const Bounds3<T> &b1, const Bounds3<T> &b2) {
    // Important: assign to pMin/pMax directly and don't run the Bounds2()
    // constructor, since it takes min/max of the points passed to it.  In
    // turn, that breaks returning an invalid bound for the case where we
    // intersect non-overlapping bounds (as we'd like to happen).
    Bounds3<T> ret;
    ret.pMin = max(b1.pMin, b2.pMin);
    ret.pMax = min(b1.pMax, b2.pMax);
    return ret;
}

template <typename T>
bool overlaps(const Bounds3<T> &b1, const Bounds3<T> &b2) {
    bool x = (b1.pMax.x >= b2.pMin.x) && (b1.pMin.x <= b2.pMax.x);
    bool y = (b1.pMax.y >= b2.pMin.y) && (b1.pMin.y <= b2.pMax.y);
    bool z = (b1.pMax.z >= b2.pMin.z) && (b1.pMin.z <= b2.pMax.z);
    return (x && y && z);
}

template <typename T>
bool inside(const Point3<T> &p, const Bounds3<T> &b) {
    return (p.x >= b.pMin.x && p.x <= b.pMax.x && p.y >= b.pMin.y &&
            p.y <= b.pMax.y && p.z >= b.pMin.z && p.z <= b.pMax.z);
}

template <typename T>
bool insideExclusive(const Point3<T> &p, const Bounds3<T> &b) {
    return (p.x >= b.pMin.x && p.x < b.pMax.x && p.y >= b.pMin.y &&
            p.y < b.pMax.y && p.z >= b.pMin.z && p.z < b.pMax.z);
}

template <typename T, typename U>
inline Bounds3<T> expand(const Bounds3<T> &b, U delta) {
    return Bounds3<T>(b.pMin - Vector3<T>(delta, delta, delta),
                      b.pMax + Vector3<T>(delta, delta, delta));
}

// Minimum squared distance from point to box; returns zero if point is
// inside.
template <typename T, typename U>
inline Float distanceSquared(const Point3<T> &p, const Bounds3<U> &b) {
    Float dx = std::max({Float(0), b.pMin.x - p.x, p.x - b.pMax.x});
    Float dy = std::max({Float(0), b.pMin.y - p.y, p.y - b.pMax.y});
    Float dz = std::max({Float(0), b.pMin.z - p.z, p.z - b.pMax.z});
    return dx * dx + dy * dy + dz * dz;
}

template <typename T, typename U>
inline Float distance(const Point3<T> &p, const Bounds3<U> &b) {
    return std::sqrt(distanceSquared(p, b));
}

template <typename T>
Bounds2<T> unionSet(const Bounds2<T> &b, const Point2<T> &p) {
    Bounds2<T> ret;
    ret.pMin = Min(b.pMin, p);
    ret.pMax = Max(b.pMax, p);
    return ret;
}

template <typename T>
Bounds2<T> unionSet(const Bounds2<T> &b, const Bounds2<T> &b2) {
    Bounds2<T> ret;
    ret.pMin = Min(b.pMin, b2.pMin);
    ret.pMax = Max(b.pMax, b2.pMax);
    return ret;
}

template <typename T>
Bounds2<T> intersect(const Bounds2<T> &b1, const Bounds2<T> &b2) {
    // Important: assign to pMin/pMax directly and don't run the Bounds2()
    // constructor, since it takes min/max of the points passed to it.  In
    // turn, that breaks returning an invalid bound for the case where we
    // intersect non-overlapping bounds (as we'd like to happen).
    Bounds2<T> ret;
    ret.pMin = Max(b1.pMin, b2.pMin);
    ret.pMax = Min(b1.pMax, b2.pMax);
    return ret;
}

template <typename T>
bool overlaps(const Bounds2<T> &ba, const Bounds2<T> &bb) {
    bool x = (ba.pMax.x >= bb.pMin.x) && (ba.pMin.x <= bb.pMax.x);
    bool y = (ba.pMax.y >= bb.pMin.y) && (ba.pMin.y <= bb.pMax.y);
    return (x && y);
}

template <typename T>
bool inside(const Point2<T> &pt, const Bounds2<T> &b) {
    return (pt.x >= b.pMin.x && pt.x <= b.pMax.x && pt.y >= b.pMin.y &&
            pt.y <= b.pMax.y);
}

template <typename T>
bool insideExclusive(const Point2<T> &pt, const Bounds2<T> &b) {
    return (pt.x >= b.pMin.x && pt.x < b.pMax.x && pt.y >= b.pMin.y &&
            pt.y < b.pMax.y);
}

template <typename T, typename U>
Bounds2<T> expand(const Bounds2<T> &b, U delta) {
    return Bounds2<T>(b.pMin - Vector2<T>(delta, delta),
                      b.pMax + Vector2<T>(delta, delta));
}

inline Float lerp(Float t, Float v1, Float v2) {
    return (1 - t) * v1 + t * v2;
}

inline bool quadratic(Float a, Float b, Float c, Float *t0, Float *t1) {
    // Find quadratic discriminant
    double discrim = (double)b * (double)b - 4 * (double)a * (double)c;
    if (discrim < 0) {
        return false;
    }
    double rootDiscrim = std::sqrt(discrim);
    // Compute quadratic _t_ values
    double q;
    if (b < 0){
        q = -.5 * (b - rootDiscrim);
    } else {
        q = -.5 * (b + rootDiscrim);
    }
    *t0 = q / a;
    *t1 = c / q;
    if (*t0 > *t1) {
        std::swap(*t0, *t1);
    }
    return true;
}

inline Float gamma(int n) {
    return (n * MachineEpsilon) / (1 - n * MachineEpsilon);
}

PALADIN_END

#endif /* mathutil_h */
