//
//  header.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/7/1.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef header_h
#define header_h

// 1字节     uint8_t
// 2字节     uint16_t
// 4字节     uint32_t
// 8字节     uint64_t

#include <iostream>
#include <assert.h>
#include <cmath>
#include <stdlib.h>

//fix 'numeric_limits' is not a member of 'std' for linux
#include <limits>
//fix 'memcpy' was not declared in this scope for linux
#include <string.h>

#include "stringprint.hpp"
#include "macro.h"


PALADIN_BEGIN
template<typename T>
class Point2;

template<typename T>
class Point3;

template<typename T>
class Vector3;

template<typename T>
class Vector2;

template<typename T>
class Normal3;

template<typename T>
class Bounds3;

template<typename T>
class Bounds2;

template<typename T>
class Direction3;

class Medium;

class Ray;

class Transform;

class Interaction;

class AnimatedTransform;

class Shape;

class MemoryArena;

class Primitive;

class BSDF;

class BSSRDF;

struct Quaternion;

struct Interaction;

struct MediumInterface;

template<typename T, int logBlockSize = 2>
class BlockedArray;

class EFloat;

PALADIN_END

inline uint32_t floatToBits(float f) {
    uint32_t ui;
    memcpy(&ui, &f, sizeof(float));
    return ui;
}

inline float bitsToFloat(uint32_t ui) {
    float f;
    memcpy(&f, &ui, sizeof(uint32_t));
    return f;
}

inline uint64_t floatToBits(double f) {
    uint64_t ui;
    memcpy(&ui, &f, sizeof(double));
    return ui;
}

inline double bitsToFloat(uint64_t ui) {
    double f;
    memcpy(&f, &ui, sizeof(uint64_t));
    return f;
}

inline float nextFloatUp(float v) {
    if (std::isinf(v) && v > 0.) return v;
    if (v == -0.f) v = 0.f;
    uint32_t ui = floatToBits(v);
    if (v >= 0)
        ++ui;
    else
        --ui;
    return bitsToFloat(ui);
}

inline float nextFloatDown(float v) {
    if (std::isinf(v) && v < 0.) return v;
    if (v == 0.f) v = -0.f;
    uint32_t ui = floatToBits(v);
    if (v > 0)
        --ui;
    else
        ++ui;
    return bitsToFloat(ui);
}

inline double nextFloatUp(double v, int delta = 1) {
    if (std::isinf(v) && v > 0.) return v;
    if (v == -0.f) v = 0.f;
    uint64_t ui = floatToBits(v);
    if (v >= 0.)
        ui += delta;
    else
        ui -= delta;
    return bitsToFloat(ui);
}

inline double nextFloatDown(double v, int delta = 1) {
    if (std::isinf(v) && v < 0.) return v;
    if (v == 0.f) v = -0.f;
    uint64_t ui = floatToBits(v);
    if (v > 0.)
        ui -= delta;
    else
        ui += delta;
    return bitsToFloat(ui);
}

inline Float gammaCorrect(Float value) {
    if (value <= 0.0031308f) return 12.92f * value;
    return 1.055f * std::pow(value, (Float)(1.f / 2.4f)) - 0.055f;
}

inline Float inverseGammaCorrect(Float value) {
    if (value <= 0.04045f) return value * 1.f / 12.92f;
    return std::pow((value + 0.055f) * 1.f / 1.055f, (Float)2.4f);
}

template <typename T>
inline T mod(T a, T b) {
    T result = a - (a / b) * b;
    return (T)((result < 0) ? result + b : result);
}

template <>
inline Float mod(Float a, Float b) {
    return std::fmod(a, b);
}

// std的log是以e为底数 log2(x) = lnx / ln2
inline Float Log2(Float x) {
    const Float invLog2 = 1.442695040888963387004650940071;
    return std::log(x) * invLog2;
}

template <typename T>
inline CONSTEXPR bool isPowerOf2(T v) {
    return v && !(v & (v - 1));
}

#include "mathutil.h"
#include "vector.h"
#include "point.h"
#include "ray.h"
#include "bounds.h"
#include "memory.hpp"
#include "errfloat.h"

#endif /* header_h */










