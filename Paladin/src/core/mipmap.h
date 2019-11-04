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
#include "core/texture.hpp"
#include "math/bounds.h"

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
    MIPMap(const Point2i &resolution, const T *img, bool doTri = true,
           Float maxAniso = 8.f, ImageWrap wrapMode = ImageWrap::Repeat)
    : _doTrilinear(doTri),
    _maxAnisotropy(maxAniso),
    _wrapMode(wrapMode),
    _resolution(resolution) {
        
        std::unique_ptr<T[]> resampledImage = nullptr;
        // 如果s，t两个方向有一个方向的分辨率不是2的整数次幂，则重采样，增加采样率提高到2的整数次幂
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
            // 处理t方向上的时候需要一些临时缓存来防止污染resampledImage中的数据
            // 临时空间需要手动删除            
            std::vector<T *> resampleBufs;
            int nThreads = maxThreadIndex();
            for (int i = 0; i < nThreads; ++i) {
                resampleBufs.push_back(new T[resPow2[1]]);
            }
            parallelFor([&](int s) {
                // 保存临时列数据
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
                // 把最新数据填充到resampledImage中
                for (int t = 0; t < resPow2[1]; ++t) {
                    resampledImage[t * resPow2[0] + s] = clamp(workData[t]);
                }
            }, resPow2[0], 32);
            for (auto ptr : resampleBufs) {
                delete[] ptr;
            }
            _resolution = resPow2;
        }

        int nLevels = 1 + Log2Int(std::max(resolution[0], resolution[1]));
        _pyramid.resize(nLevels);

        _pyramid[0].reset(
            new BlockedArray<T>(resolution[0], resolution[1],
                            resampledImage ? resampledImage.get() : img));

        for (int i = 1; i < nLevels; ++i) {
            int sRes = std::max(1, _pyramid[i - 1]->uSize() / 2);
            int tRes = std::max(1, _pyramid[i - 1]->vSize() / 2);
            _pyramid[i].reset(new BlockedArray<T>(sRes, tRes));
            // 并行处理，逐行执行
            parallelFor([&](int t) {
                for (int s = 0; s < sRes; ++s) {
                    // 对应位置的四个像素取平均值
                    (*_pyramid[i])(s, t) = .25f * 
                            (texel(i - 1, 2 * s, 2 * t) +
                            texel(i - 1, 2 * s + 1, 2 * t) +
                            texel(i - 1, 2 * s, 2 * t + 1) +
                            texel(i - 1, 2 * s + 1, 2 * t + 1));
                }
            }, tRes, 16);
        }
        // 如果没有初始化过ewa权重查询表的话，则初始化
        // 按照正态分布计算权重查询表
        if (_weightLut[0] == 0.) {
            for (int i = 0; i < WeightLUTSize; ++i) {
                Float alpha = 2;
                Float r2 = Float(i) / Float(WeightLUTSize - 1);
                _weightLut[i] = std::exp(-alpha * r2) - std::exp(-alpha);
            }
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
    
    const T &texel(int level, int s, int t) const {
        CHECK_LT(level, _pyramid.size());
        const BlockedArray<T> &l = *_pyramid[level];
        switch (_wrapMode) {
            case ImageWrap::Repeat:
                s = Mod(s, l.uSize());
                t = Mod(t, l.vSize());
                break;
            case ImageWrap::Clamp:
                s = clamp(s, 0, l.uSize() - 1);
                t = clamp(t, 0, l.vSize() - 1);
                break;
            case ImageWrap::Black:
                static const T black(0.0f);
                if (s < 0 || s >= l.uSize() || t < 0 || t > l.vSize()) {
                    return black;
                }
                break;
        }
        return l(s, t);
    }

    /**
     * 根据宽度纹理值  
     * @param  st    纹理坐标
     * @param  width 过滤宽度
     * @return       [description]
     */
    T lookup(const Point2f &st, Float width = 0.f) const {
        // 根据宽度找到对应的mipmap级别
        // width越大，对应的纹理级别越高，分辨率越低
        // 1/width = 2^(nLevels - 1 - level)
        Float level = levels() - 1 + Log2(std::max(width, (Float)1e-8));

        if (level < 0) {
            // 如果分辨率最大的纹理也不能满足需求
            return triangle(0, st);
        } else if (level >= levels() - 1) {
            // 如果已经取到了金字塔顶端的纹理，则直接取值
            return texel(levels() - 1, 0, 0);
        } else {
            // 如果level范围在纹理金字塔的范围内
            int iLevel = std::floor(level);
            Float delta = level - iLevel;
            // 对相邻两个级别的纹理取插值
            return lerp(delta, triangle(iLevel, st), triangle(iLevel + 1, st));
        }
    }
    
    /**
     * 纹理查询函数
     * 通过st纹理以及x,y方向的偏导数去选择mipmap的级别
     * 最简单的方式是三角过滤：
     *     通过各个方向偏导数，找到跨度最大的方向，作为过滤宽度
     *     
     * 但这样会引起一个问题，如果角度十分倾斜的时候，
     * 屏幕空间x方向的在纹理空间采样跨度可能很小，但y方向在纹理空间采样的跨度可能很大
     * 如果一律按照最大跨度去处理，效果可能不是很好，所以产生了另一个比较复杂的算法
     * 参考资料 http://www.pbr-book.org/3ed-2018/Texture/Image_Texture.html#EllipticallyWeightedAverage
     * Elliptically Weighted Average (ewa):
     *     x方向的采样跨度与y方向的跨度不同，可以将这样的情况看成一个椭圆
     * 
     * @param  st    纹理坐标
     * @param  dst0  dstdx
     * @param  dst1  dstdy
     * @return       [description]
     */
    T lookup(const Point2f &st, Vector2f dst0, Vector2f dst1) const {
        using namespace std;
        if (_doTrilinear) {
            Float width = std::max(std::max(std::abs(dst0[0]), 
                                    std::abs(dst0[1])), 
                            std::max(std::abs(dst1[0]), 
                                    std::abs(dst1[1])));
            return lookup(st, width);
        }
        // ewa
        // 找到椭圆较长的轴
        // 保证dst0是主轴
        if (dst0.lengthSquared() < dst1.lengthSquared()) {
            std::swap(dst0, dst1);
        }
        Float majorLength = dst0.length();
        Float minorLength = dst1.length();

        // 如果有偏心率过大，椭圆极度瘦长，则有很大的范围需要过滤
        // 为了避免这种大计算量的出现
        // 我们需要限制椭圆偏心率，扩大短轴(结果会导致一些模糊，但不明显，能接受)
        // 如果短轴过短，则扩大短轴，使之满足最大各向异性之比
        if (minorLength * _maxAnisotropy < majorLength) {
            Float scale = majorLength / (minorLength * _maxAnisotropy);
            dst1 = dst1 * scale;
            minorLength = minorLength * scale;
        }

        if (minorLength == 0) {
            return triangle(0, st);
        }
        Float lv = levels() - (Float)1 + Log2(minorLength);
        Float lod = std::max((Float)0, lv);
        int iLod = std::floor(lod);
        return lerp(lod - iLod,
                    EWA(iLod, st, dst0, dst1),
                    EWA(iLod + 1, st, dst0, dst1));
    }
    
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
    
    T triangle(int level, const Point2f &st) const {
        level = clamp(level, 0, levels() - 1);
        // 离散坐标转为连续坐标
        Float s = st.s * _pyramid[level]->uSize() - 0.5f;
        Float t = st.t * _pyramid[level]->vSize() - 0.5f;
        int s0 = std::floor(s);
        int t0 = std::floor(t);
        Float ds = s - s0;
        Float dt = t - t0;
        // 相当于双线性插值
        return (1 - ds) * (1 - dt) * texel(level, s0,   t0) +
               (1 - ds) * dt       * texel(level, s0,   t0+1) +
               ds       * (1 - dt) * texel(level, s0+1, t0) +
               ds       * dt       * texel(level, s0+1, t0+1);
    }
    
    /**
     * 椭圆加权平均函数，基本思路如下
     * 1.构造一个以st坐标为原点，dst0为长半轴，dst1向量为短半轴的椭圆
     * 2.计算出椭圆在纹理空间中的AABB
     * 3.遍历AABB中的所有纹理像素点，对椭圆范围内的所有像素进行过滤
     * 相当于在椭圆范围内按照对应权重进行高斯过滤
     * @param  level 纹理级别
     * @param  st    st坐标
     * @param  dst0  可以认为是椭圆长半轴向量
     * @param  dst1  椭圆短半轴向量
     * @return       [description]
     */
    T EWA(int level, Point2f st, Vector2f dst0, Vector2f dst1) const {
        if (level >= levels()) {
            return texel(levels() - 1, 0, 0);
        }

        // 先把st坐标从[0,1)范围转到对应级别纹理的分辨率上
        // 对应的偏导数也要进行转换
        st.s = st.s * _pyramid[level]->uSize() - 0.5f;
        st.t = st.t * _pyramid[level]->vSize() - 0.5f;
        dst0 = dst0 * _pyramid[level]->uSize();
        dst1 = dst1 * _pyramid[level]->vSize();

        // 开始计算椭圆方程
        // 高中数学就学过椭圆方程啦，做个转换得到如下形式
        // 并且将椭圆移动到原点之后得到如下方程
        // e(s,t) = A s^2 + B s t + C t^2 < F
        // 整理一下得到新的椭圆e的表达式
        // e(s,t) = (A/F) s^2 + (B/F) s t + (C/F) t^2 < 1
        // 以上不等式的(s,t)的点集表示在椭圆内的点
        // 求系数ABCF的推导方式就暂时不去管了，有时间就手动推导一把todo
        // 论文为[Heck89]Fundamentals of Texture Mapping and Image 
        Float A = dst0[1] * dst0[1] + dst1[1] * dst1[1] + 1;
        Float B = -2 * (dst0[0] * dst0[1] + dst1[0] * dst1[1]);
        Float C = dst0[0] * dst0[0] + dst1[0] * dst1[0] + 1;
        Float invF = 1 / (A * C - B * B * 0.25f);        
        A *= invF;
        B *= invF;
        C *= invF;

        // 计算出椭圆方程之后，计算出椭圆在离散纹理空间中的AABB
        // 其实就是计算出椭圆方程分的s与t的最大值与最小值，计算方法如下
        // 偏导数dt/ds为0时为t的极值，ds/dt为0时为s的极值
        // 求了极值之后
        // 推导过程就不写了，太特么复杂了，不过可以肯定的是我是掌握了的😂
        // 直接贴代码吧
        // 注意：由于椭圆是经过平移的，所以求出的aabb也需要平移回到(s,t)点
        Float det = -B * B + 4 * A * C;
        Float invDet = 1 / det;
        Float uSqrt = std::sqrt(det * C), vSqrt = std::sqrt(A * det);
        int s0 = std::ceil (st[0] - 2 * invDet * uSqrt);
        int s1 = std::floor(st[0] + 2 * invDet * uSqrt);
        int t0 = std::ceil (st[1] - 2 * invDet * vSqrt);
        int t1 = std::floor(st[1] + 2 * invDet * vSqrt);

        // 遍历AABB内的所有纹理像素点，对于在椭圆内的点进行高斯过滤
        T sum(0.0f);
        Float sumWts = 0;
        for (int it = t0; it <= t1; ++it) {
            Float tt = it - st.t;
            for (int is = s0; is <= s1; ++is) {
                Float ss = is - st.s;
                Float r2 = A * ss * ss + B * ss * tt + C * tt * tt;
                // e(s,t) = A s^2 + B s t + C t^2 < 1
                if (r2 < 1) {
                    // 找到满足上述方程的点进行过滤
                    int index = std::min((int)(r2 * WeightLUTSize), 
                                                WeightLUTSize - 1);
                    Float weight = _weightLut[index];
                    sum += texel(level, is, it) * weight;
                    sumWts += weight;
                }
            }
        }
        return sum / sumWts;
    }
    
    // 是否为三线性插值
    const bool _doTrilinear;

    // 各向异性的最大比例，可以理解为椭圆的最大偏心率
    // 详见lookup函数注释
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

template <typename T>
Float MIPMap<T>::_weightLut[WeightLUTSize];

PALADIN_END

#endif /* mipmap_h */
