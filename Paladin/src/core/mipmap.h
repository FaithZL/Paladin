//
//  mipmap.h
//  Paladin
//
//  Created by SATAN_Z on 2019/10/4.
//

#ifndef mipmap_h
#define mipmap_h

#include "header.h"

PALADIN_BEGIN

enum class ImageWrap { Repeat, Black, Clamp };

struct ResampleWeight {
    int firstTexel;
    Float weight[4];
};

template <typename T>
class MIPMap {
public:
    MIPMap(const Point2i &resolution, const T *data, bool doTri = false,
           Float maxAniso = 8.f, ImageWrap wrapMode = ImageWrap::Repeat)
    : _doTrilinear(doTri),
    _maxAnisotropy(maxAniso),
    _wrapMode(wrapMode),
    _resolution(resolution) {
        
        std::unique_ptr<T[]> resampledImage = nullptr;
        if (!isPowerOf2(_resolution[0]) || !isPowerOf2(_resolution[1])) {
            Point2i resPow2(roundUpPow2(_resolution[0]), roundUpPow2(_resolution[1]));
            // 在s方向重采样
            std::unique_ptr<ResampleWeight[]> sWeights = resampleWeights(_resolution[0], resPow2[0]);
            resampledImage.reset(new T[resPow2[0] * resPow2[1]]);
        }
    }
    
    int width() const {
        return _resolution[0];
    }
    
    int height() const {
        return _resolution[1];
    }
    
    int levels() const {
        return _pyramid.size();
    }
    
    const T &texel(int level, int s, int t) const;
    
    T lookup(const Point2f &st, Float width = 0.f) const;
    
    T lookup(const Point2f &st, Vector2f dstdx, Vector2f dstdy) const;
    
private:
    
    // 是否为三线性插值
    const bool _doTrilinear;
    //
    const Float _maxAnisotropy;
    // 环绕方式
    const ImageWrap _wrapMode;
    // 分辨率
    Point2i _resolution;
    //
    std::vector<std::unique_ptr<BlockedArray<T>>> _pyramid;
    static CONSTEXPR int WeightLUTSize = 128;
    static Float _weightLut[WeightLUTSize];
};

PALADIN_END

#endif /* mipmap_h */
