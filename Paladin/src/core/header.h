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
#include <vector>

#ifdef __GNUC__
    //fix 'numeric_limits' is not a member of 'std' for linux
    #include <limits>
    //fix 'memcpy' was not declared in this scope for linux
    #include <string.h>
    //fix 'shared_ptr' is not a member of 'std' for linux
    #include <memory>
    //fix 'atomic' is not a member of 'std' for linux
    #include <atomic>
    //fix 'partition' is not a member of 'std' for linux
    #include <algorithm>
#endif

#ifdef _MSC_VER
    //fix 二进制“>>”: 没有找到接受“std::string”类型的右操作数
    //的运算符(或没有可接受的转换) for vs2015
    #include <string>
    //fix “max”: 不是“std”的成员 for vs2015
    #include <algorithm>
    //fix “shared_ptr”: 不是“std”的成员 for vs2015
    #include <memory>
    //fix “atomic”: 不是“std”的成员 for vs2015
    #include <atomic>
    
#endif

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
class AABB3;

template<typename T>
class AABB2;

template<typename T>
class Direction3;

class Medium;

class RGBSpectrum;

class SampledSpectrum;

#ifdef PALADIN_SAMPLED_SPECTRUM
typedef SampledSpectrum Spectrum;
#else
typedef RGBSpectrum Spectrum;
#endif

class Ray;

class Transform;

class AnimatedTransform;

class Shape;

class MediumInteraction;

class MemoryArena;

class Primitive;

class BSDF;

class VisibilityTester;

class Sampler;

class Scene;

class BSSRDF;

struct Quaternion;

struct Interaction;

struct MediumInterface;

template<typename T, int logBlockSize = 2>
class BlockedArray;

class EFloat;

class Light;

class AreaLight;

class Shape;

class Material;

template <typename T>
class Texture;

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

template <typename Predicate>
int findInterval(int size, const Predicate &pred) {
    int first = 0, len = size;
    while (len > 0) {
        int half = len >> 1, middle = first + half;
        // Bisect range based on value of _pred_ at _middle_
        if (pred(middle)) {
            first = middle + 1;
            len -= half + 1;
        } else
            len = half;
    }
    return paladin::clamp(first - 1, 0, size - 2);
}

#include "vector.h"
#include "point.h"
#include "ray.h"
#include "bounds.h"
#include "memory.hpp"
#include "errfloat.h"

#endif /* header_h */










