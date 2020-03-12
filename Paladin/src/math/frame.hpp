//
//  frame.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/3/12.
//

#ifndef frame_hpp
#define frame_hpp

#include "core/header.h"

PALADIN_BEGIN


struct Frame {
    
    inline Frame() { }

    inline Frame(const Vector3f &s, const Vector3f &t, const Normal3f &n)
    : s(s), t(t), n(n) {
        
    }
    
    inline Frame(const Vector3f &x, const Vector3f &y, const Vector3f &z)
    : s(x), t(y), n(z) {
         
    }

    inline Frame(const Vector3f &n) : n(n) {
        coordinateSystem(n, &s, &t);
    }
    
    inline Vector3f toLocal(const Vector3f &v) const {
        return Vector3f(
            dot(v, s),
            dot(v, t),
            dot(v, n)
        );
    }
    
    inline Vector3f toWorld(const Vector3f &v) const {
        return s * v.x + t * v.y + Vector3f(n * v.z);
    }
    
    inline static Float cosTheta2(const Vector3f &v) {
        return v.z * v.z;
    }
    
    inline static Float cosTheta(const Vector3f &v) {
        return v.z;
    }
    
    inline static Vector2f uv(const Vector3f &v) {
        return Vector2f(v.x, v.y);
    }
    
    inline static Float sinTheta2(const Vector3f &v) {
        return 1.0f - v.z * v.z;
    }
    
    inline static Float sinTheta(const Vector3f &v) {
        Float temp = sinTheta2(v);
        if (temp <= 0.0f) {
            return 0.0f;
        }
        return std::sqrt(temp);
    }
    
    inline static Float tanTheta(const Vector3f &v) {
        Float temp = 1 - v.z*v.z;
        if (temp <= 0.0f) {
            return 0.0f;
        }
        return std::sqrt(temp) / v.z;
    }
    
    inline static Float tanTheta2(const Vector3f &v) {
        Float temp = 1 - v.z*v.z;
        if (temp <= 0.0f) {
            return 0.0f;
        }
        return temp / (v.z * v.z);
    }
    
    inline static Float sinPhi(const Vector3f &v) {
        Float sinTheta = Frame::sinTheta(v);
        if (sinTheta == 0.0f)
            return 1.0f;
        return clamp(v.y / sinTheta, (Float) -1.0f, (Float) 1.0f);
    }

    inline static Float cosPhi(const Vector3f &v) {
        Float sinTheta = Frame::sinTheta(v);
        if (sinTheta == 0.0f)
            return 1.0f;
        return clamp(v.x / sinTheta, (Float) -1.0f, (Float) 1.0f);
    }
    
    inline static Float sinPhi2(const Vector3f &v) {
        return clamp(v.y * v.y / sinTheta2(v), (Float) 0.0f, (Float) 1.0f);
    }

    inline static Float cosPhi2(const Vector3f &v) {
        return clamp(v.x * v.x / sinTheta2(v), (Float) 0.0f, (Float) 1.0f);
    }
    
    inline bool operator == (const Frame &frame) const {
        return frame.s == s && frame.t == t && frame.n == n;
    }
    
    inline bool operator != (const Frame &frame) const {
        return !operator == (frame);
    }

private:
    
    Vector3f s,t;
    
    Normal3f n;
};


PALADIN_END

#endif /* frame_hpp */
