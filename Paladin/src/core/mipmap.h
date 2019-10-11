//
//  mipmap.h
//  Paladin
//
//  Created by SATAN_Z on 2019/10/4.
//

#ifndef mipmap_h
#define mipmap_h

#include "header.h"
#include "tools/parallel.hpp"
#include "core/spectrum.hpp"

PALADIN_BEGIN

enum class ImageWrap { Repeat, Black, Clamp };

// 重采样的权重
struct ResampleWeight {
    // 第一个纹理像素的索引
    int firstTexel;
    Float weight[4];
};

template <typename T>
class MIPMap {
public:
    MIPMap(const Point2i &resolution, const T *img, bool doTri = false,
           Float maxAniso = 8.f, ImageWrap wrapMode = ImageWrap::Repeat)
    : _doTrilinear(doTri),
    _maxAnisotropy(maxAniso),
    _wrapMode(wrapMode),
    _resolution(resolution) {
        
        std::unique_ptr<T[]> resampledImage = nullptr;
        if (!isPowerOf2(_resolution[0]) || !isPowerOf2(_resolution[1])) {
            Point2i resPow2(roundUpPow2(_resolution[0]), roundUpPow2(_resolution[1]));
            // 在s方向重采样
            // 获取到一系列的sWeights对象之后，重建出新的分辨率
            std::unique_ptr<ResampleWeight[]> sWeights = resampleWeights(_resolution[0], resPow2[0]);
            resampledImage.reset(new T[resPow2[0] * resPow2[1]]);
            
            parallelFor([&](int t) {
                for (int s = 0; s < resPow2[0]; ++s) {
                    
                    resampledImage[t * resPow2[0] + s] = 0.f;
                    for (int j = 0; j < 4; ++j) {
                        int origS = sWeights[s].firstTexel + j;
                        if (wrapMode == ImageWrap::Repeat) {
                            origS = Mod(origS, resolution[0]);
                        } else if (wrapMode == ImageWrap::Clamp) {
                            origS = clamp(origS, 0, resolution[0] - 1);
                        }
                        if (origS >= 0 && origS < (int)resolution[0]) {
                            resampledImage[t * resPow2[0] + s] +=
                                sWeights[s].weight[j] *
                                img[t * resolution[0] + origS];
                        }
                    }
                }
            }, resolution[1], 16);
            
            std::unique_ptr<ResampleWeight[]> tWeights = resampleWeights(resolution[1], resPow2[1]);
            std::vector<T *> resampleBufs;
            // 处理t方向上的时候需要一些临时缓存来防止污染resampledImage中的数据
            // 临时空间需要手动删除            
            int nThreads = maxThreadIndex();
            for (int i = 0; i < nThreads; ++i) {
                resampleBufs.push_back(new T[resPow2[1]]);
            }
            parallelFor([&](int s) {
                T *workData = resampleBufs[ThreadIndex];
                for (int t = 0; t < resPow2[1]; ++t) {
                    workData[t] = 0.f;
                    for (int j = 0; j < 4; ++j) {
                        int offset = tWeights[t].firstTexel + j;
                        if (wrapMode == ImageWrap::Repeat) {
                            offset = Mod(offset, resolution[1]);
                        } else if (wrapMode == ImageWrap::Clamp) {
                            offset = clamp(offset, 0, (int)resolution[1] - 1);
                        }
                        if (offset >= 0 && offset < (int)resolution[1]) {
                            workData[t] += tWeights[t].weight[j] *
                                resampledImage[offset * resPow2[0] + s];
                        }
                    }
                }
                for (int t = 0; t < resPow2[1]; ++t) {
                    resampledImage[t * resPow2[0] + s] = clamp(workData[t]);
                }
            }, resPow2[0], 32);
            for (auto ptr : resampleBufs) {
                delete[] ptr;
            }
            _resolution = resPow2;
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
    
    /**
     * 重采样函数，返回newRes个ResampleWeight对象
     * @param oldRes 旧分辨率
     * @param newRes 新分辨率
     */
    std::unique_ptr<ResampleWeight[]> resampleWeights(int oldRes, int newRes) {
        CHECK_GE(newRes, oldRes);
        std::unique_ptr<ResampleWeight[]> ret(new ResampleWeight[newRes]);
        // 过滤宽度，默认2.0
        Float filterwidth = 2.0f;
        for (int i = 0; i < newRes; ++i) {
            // 离散坐标转化为连续坐标，都取像素中点，所以加上0.5
            Float center = (i + 0.5f) * oldRes / newRes;
            // todo添加图示
            ret[i].firstTexel = std::floor((center - filterwidth) + 0.5);
            Float weightSum = 0;
            for (int j = 0; j < 4; ++j) {
                Float pos = ret[i].firstTexel + j + 0.5;
                ret[i].weight[j] = lanczos((pos - center) / filterwidth, 2);
                weightSum += ret[i].weight[j];
            }
            // 四个权重值之和可能不为1，为了确保新的样本不比原始样本更加亮或暗，则需要归一化
            Float invSumWts = 1 / weightSum;
            for (int j = 0; j < 4; ++j) {
                ret[i].weight[j] *= invSumWts;
            }
        }
        return ret;
    }
    
    Float clamp(Float v) {
        return clamp(v, 0.f, Infinity);
    }
    
    RGBSpectrum clamp(const RGBSpectrum &v) {
        return v.Clamp(0.f, Infinity);
    }
    
    SampledSpectrum clamp(const SampledSpectrum &v) {
        return v.Clamp(0.f, Infinity);
    }
    
    T triangle(int level, const Point2f &st) const;
    
    T EWA(int level, Point2f st, Vector2f dst0, Vector2f dst1) const;
    
    // 是否为三线性插值
    const bool _doTrilinear;
    // 各向异性的最大比例
    const Float _maxAnisotropy;
    // 环绕方式
    const ImageWrap _wrapMode;
    // 分辨率
    Point2i _resolution;
    // 多级纹理金字塔
    std::vector<std::unique_ptr<BlockedArray<T>>> _pyramid;
    static CONSTEXPR int WeightLUTSize = 128;
    static Float _weightLut[WeightLUTSize];
};

PALADIN_END

#endif /* mipmap_h */
