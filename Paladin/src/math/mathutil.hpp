//
//  mathutil.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/7/11.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef mathutil_hpp
#define mathutil_hpp

#include "header.h"

PALADIN_BEGIN

template <typename T>
inline bool isNaN(const T x);

template <>
inline bool isNaN(const int x);

inline Float degree2radian(Float deg);

inline Float radian2degree(Float rad);

template <typename T, typename U>
inline Vector3<T> operator*(U s, const Vector3<T> &v);

template <typename T>
Vector3<T> abs(const Vector3<T> &v);

template <typename T>
inline T dot(const Vector3<T> &v1, const Vector3<T> &v2);

template <typename T>
inline T absDot(const Vector3<T> &v1, const Vector3<T> &v2);

template <typename T>
Vector3<T> cross(const Vector3<T> &v1, const Vector3<T> &v2);

template <typename T>
Vector3<T> cross(const Vector3<T> &v1, const Normal3<T> &v2);

template <typename T>
Vector3<T> cross(const Normal3<T> &v1, const Vector3<T> &v2);

template <typename T>
inline Vector3<T> normalize(const Vector3<T> &v);

template <typename T>
T minComponent(const Vector3<T> &v);

template <typename T>
T maxComponent(const Vector3<T> &v);

template <typename T>
int maxDimension(const Vector3<T> &v);

template <typename T>
Vector3<T> min(const Vector3<T> &p1, const Vector3<T> &p2);

template <typename T>
Vector3<T> max(const Vector3<T> &p1, const Vector3<T> &p2);

template <typename T>
Vector3<T> permute(const Vector3<T> &v, int x, int y, int z);

template <typename T>
inline void coordinateSystem(const Vector3<T> &v1, Vector3<T> *v2,
                             Vector3<T> *v3);

template <typename T, typename U>
inline Vector2<T> operator*(U f, const Vector2<T> &v);

template <typename T>
inline Float dot(const Vector2<T> &v1, const Vector2<T> &v2);

template <typename T>
inline Float absDot(const Vector2<T> &v1, const Vector2<T> &v2);

template <typename T>
inline Vector2<T> normalize(const Vector2<T> &v);

template <typename T>
Vector2<T> abs(const Vector2<T> &v);

template <typename T>
inline Float distance(const Point3<T> &p1, const Point3<T> &p2);


template <typename T>
inline Float distanceSquared(const Point3<T> &p1, const Point3<T> &p2);

template <typename T, typename U>
inline Point3<T> operator*(U f, const Point3<T> &p);

template <typename T>
Point3<T> lerp(Float t, const Point3<T> &p0, const Point3<T> &p1);

template <typename T>
Point3<T> min(const Point3<T> &p1, const Point3<T> &p2);

template <typename T>
Point3<T> max(const Point3<T> &p1, const Point3<T> &p2);

template <typename T>
Point3<T> floor(const Point3<T> &p);

template <typename T>
Point3<T> ceil(const Point3<T> &p);

template <typename T>
Point3<T> abs(const Point3<T> &p);

template <typename T>
inline Float distance(const Point2<T> &p1, const Point2<T> &p2);

template <typename T>
inline Float distanceSquared(const Point2<T> &p1, const Point2<T> &p2);

template <typename T, typename U>
inline Point2<T> operator*(U f, const Point2<T> &p);

template <typename T>
Point2<T> floor(const Point2<T> &p);

template <typename T>
Point2<T> ceil(const Point2<T> &p);

template <typename T>
Point2<T> lerp(Float t, const Point2<T> &v0, const Point2<T> &v1);

template <typename T>
Point2<T> min(const Point2<T> &pa, const Point2<T> &pb);

template <typename T>
Point2<T> max(const Point2<T> &pa, const Point2<T> &pb);

template <typename T>
Point3<T> permute(const Point3<T> &p, int x, int y, int z);

template <typename T, typename U>
inline Normal3<T> operator*(U f, const Normal3<T> &n);

template <typename T>
Normal3<T> normalize(const Normal3<T> &n);

template <typename T>
inline T dot(const Normal3<T> &n1, const Vector3<T> &v2);

template <typename T>
inline T dot(const Vector3<T> &v1, const Normal3<T> &n2);

template <typename T>
inline T dot(const Normal3<T> &n1, const Normal3<T> &n2);

template <typename T>
inline T absDot(const Normal3<T> &n1, const Vector3<T> &v2);

template <typename T>
inline T absDot(const Vector3<T> &v1, const Normal3<T> &n2);

template <typename T>
inline T absDot(const Normal3<T> &n1, const Normal3<T> &n2);

template <typename T>
inline Normal3<T> faceforward(const Normal3<T> &n, const Vector3<T> &v);

template <typename T>
inline Normal3<T> faceforward(const Normal3<T> &n, const Normal3<T> &n2);

template <typename T>
inline Vector3<T> faceforward(const Vector3<T> &v, const Vector3<T> &v2);

template <typename T>
inline Vector3<T> faceforward(const Vector3<T> &v, const Normal3<T> &n2);

template <typename T>
Normal3<T> abs(const Normal3<T> &v);


template <typename T>
Bounds3<T> unionSet(const Bounds3<T> &b, const Point3<T> &p);

template <typename T>
Bounds3<T> unionSet(const Bounds3<T> &b1, const Bounds3<T> &b2);

template <typename T>
Bounds3<T> intersect(const Bounds3<T> &b1, const Bounds3<T> &b2);

template <typename T>
bool overlaps(const Bounds3<T> &b1, const Bounds3<T> &b2);

template <typename T>
bool inside(const Point3<T> &p, const Bounds3<T> &b);

template <typename T>
bool insideExclusive(const Point3<T> &p, const Bounds3<T> &b);

template <typename T, typename U>
inline Bounds3<T> expand(const Bounds3<T> &b, U delta);

// Minimum squared distance from point to box; returns zero if point is
// inside.
template <typename T, typename U>
inline Float distanceSquared(const Point3<T> &p, const Bounds3<U> &b);

template <typename T, typename U>
inline Float distance(const Point3<T> &p, const Bounds3<U> &b);

template <typename T>
Bounds2<T> unionSet(const Bounds2<T> &b, const Point2<T> &p);

template <typename T>
Bounds2<T> unionSet(const Bounds2<T> &b, const Bounds2<T> &b2);

template <typename T>
Bounds2<T> intersect(const Bounds2<T> &b1, const Bounds2<T> &b2);

template <typename T>
bool overlaps(const Bounds2<T> &ba, const Bounds2<T> &bb);

template <typename T>
bool inside(const Point2<T> &pt, const Bounds2<T> &b);

template <typename T>
bool insideExclusive(const Point2<T> &pt, const Bounds2<T> &b);

template <typename T, typename U>
Bounds2<T> expand(const Bounds2<T> &b, U delta);

inline Float lerp(Float t, Float v1, Float v2);

inline bool solveLinearSystem2x2(const Float A[2][2], const Float B[2], Float *x0, Float *x1);

inline bool quadratic(Float a, Float b, Float c, Float *t0, Float *t1);

inline Float gamma(int n);


PALADIN_END

#endif /* mathutil_hpp */
