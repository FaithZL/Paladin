//
//  light.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef light_hpp
#define light_hpp

#include "core/header.h"
#include "core/interaction.hpp"
#include "math/transform.hpp"
#include "core/spectrum.hpp"
#include "core/cobject.h"

PALADIN_BEGIN

/**
 * 先来简单介绍一下黑体辐射
 * 
 * 黑体是一种完美的发射器:它能尽可能有效地将电能转化为电磁辐射。
 * 虽然真正的黑体在物理上是不存在的，但一些发射器表现出接近黑体的行为。
 * 黑体的发射也有一个有用的封闭表达式(解析解)
 * 它是参数为温度和波长的函数，这对非黑体发射体的建模很有用。
 * 
 * 黑体之所以这样命名，是因为它吸收所有的入射光，没有任何反射
 * 看起来就是全黑，从直观上看，完美吸收体也是完美发射体的原因在于，吸收是发射的反向操作。
 * 因此，如果时间倒转，所有被完全吸收的能量都会被完全有效地重新释放
 *
 * 普朗克定律给出了黑体发射的辐射率，它是波长和温度的函数，单位是开尔文:
 *
 *                        2 h c^2
 *    Le(λ,T) = -----------------------------    12.1
 *                λ^5 [e^(h c / λ kb T) - 1]
 *
 *   其中 h 为普兰克常数 h = 6.62606957 * 10^34 Js
 *   c 为光在真空中传播的速度 c = 299792458 m/s
 *   kb 为玻尔兹曼常数 kb = 1.3806488 * 10^-23 J/K
 *
 * 斯蒂芬-波尔兹曼定律给出了黑体辐射源在点p处的辐射出度
 *
 *   M(p)=σT^4
 *
 *   其中σ为斯蒂芬-波尔兹曼常量σ = 5.67032 × 10^−8 Wm^−2 K^−4
 *
 * 由于黑体发出的功率随着温度的升高而迅速增加，
 * 因此计算在任何波长处SPD的最大值为1的黑体的归一化SPD也是有用的。
 * 这很容易用维恩位移定律来做，它给出了在给定温度下黑体发射最大的波长
 *              b
 *     λmax = -----
 *              T
 *     其中b为维恩位移常量 b = 2.8977721 × 10^−3 mK.
 *
 *
 * 非黑体的发射行为由基尔霍夫定律描述，
 * 基尔霍夫定律说，在任何频率的发射辐射分布等于在该频率的一个完美黑体辐射，
 * 乘以该频率的入射辐射被物体吸收的比例。(这个关系是由假定物体处于热平衡得出的。)
 * 吸收的辐射率等于1减去反射的辐射率，所以发射的辐射率是
 *
 *     Le'(T, ω, λ) = Le(T, λ)(1 − ρhd(ω)),
 *
 *     Le(T,λ)为发出的辐射度由普朗克定律,给出方程(12.1),
 *     ρhd(ω)是hemispherical-directional反射，详见bxdf类的rho_hh函数注释。
 * 
 */


enum class LightFlags {
    // 点光源
    DeltaPosition = 1,
    // 方向光，只有一个方向
    DeltaDirection = 2,
    // 面光源
    Area = 4,
    Infinite = 8
};

inline bool isDeltaLight(int flags) {
    return flags & (int)LightFlags::DeltaPosition ||
           flags & (int)LightFlags::DeltaDirection;
}

class Light : public CObject {
    
public:
    
    virtual ~Light() {

    }

    Light(int flags, const Transform * LightToWorld,
          const MediumInterface &mediumInterface, int nSamples = 1)
        
    : flags(flags),
    nSamples(std::max(1, nSamples)),
    mediumInterface(mediumInterface),
    _lightToWorld(LightToWorld),
    _worldToLight(LightToWorld->getInverse_ptr()) {

    }
    
    /**
     * 返回从指定位置ref随机采样光源表面的点得到的辐射度
     * @param  ref 指定位置
     * @param  u   2维随机变量
     * @param  wi  返回：光线射向ref的方向向量
     * @param  pdf 返回：对应随机变量的pdf值
     * @param  vis 可见测试器
     * @return     辐射度
     */
    virtual Spectrum sample_Li(const Interaction &ref, const Point2f &u,
                               Vector3f *wi, Float *pdf,
                               VisibilityTester *vis) const = 0;

    // 辐射通量，也就是功率
    virtual Spectrum power() const = 0;

    inline bool isDelta() const {
        return isDeltaLight(flags);
    }

    virtual void preprocess(const Scene &scene) {

    }
    
    /**
     * 根据Le分布采样光源
     * @param  u1     用于光源表面
     * @param  u2     用于生成方向
     * @param  time   时间
     * @param  ray    返回生成的ray
     * @param  nLight 光源表面法线
     * @param  pdfPos 位置PDF
     * @param  pdfDir 方向PDF
     * @return        [description]
     */
    virtual Spectrum sample_Le(const Point2f &u1, const Point2f &u2,
                            Float time, Ray *ray, Normal3f *nLight,
                            Float *pdfPos, Float *pdfDir) const = 0;
    
    virtual void pdf_Le(const Ray &ray, const Normal3f &nLight,
                        Float *pdfPos, Float *pdfDir) const = 0;

    virtual Spectrum Le(const RayDifferential &r) const {
        return Spectrum(0.f);
    }

    // 返回在ref处采样光源时，对应的pdf函数值
    // 用于估计直接光照时，采样bsdf时生成的wi方向，对应的pdf函数值
    virtual Float pdf_Li(const Interaction &ref, const Vector3f &wi) const = 0;
    
    virtual Float pdf_Li(const DirectSamplingRecord &) const {
        NotImplementedError("pdf_li");
    }
    
    // // 双向方法需要用的函数，暂时不理
    // virtual Spectrum sampleLe(const Point2f &u1, const Point2f &u2, Float time,
    //                            Ray *ray, Normal3f *nLight, Float *pdfPos,
    //                            Float *pdfDir) const = 0;
    
    // // 双向方法用的函数，暂时不理
    // virtual void pdfLe(const Ray &ray, const Normal3f &nLight, Float *pdfPos,
    //                     Float *pdfDir) const = 0;
    // LightFlags
    const int flags;
    // 为了计算soft shadow的采样数量
    const int nSamples;

    const MediumInterface mediumInterface;
    
protected:

    const Transform * _lightToWorld;
    const Transform * _worldToLight;
};

class AreaLight : public Light {
public:
    AreaLight(const Transform * LightToWorld,
            const MediumInterface &mi,
            int nSamples)
    :Light((int)LightFlags::Area, LightToWorld, mi, nSamples) {

    }

    virtual Spectrum L(const Interaction &intr, const Vector3f &w) const = 0;
};

/**
 * 可见测试器
 * 测试两个指定位置之间有没有阻挡
 */
class VisibilityTester {
public:
    VisibilityTester() {
        
    }
    
    VisibilityTester(const Interaction &p0, const Interaction &p1)
    : _p0(p0), _p1(p1) {
        
    }
    
    const Interaction &P0() const {
        return _p0;
    }
    
    const Interaction &P1() const {
        return _p1;
    }
    
    /**
     * 用于测试有无遮挡
     * @param  scene 场景对象
     * @return       无遮挡时返回true
     */
    bool unoccluded(const Scene &scene) const;
    
    Spectrum Tr(const Scene &scene, Sampler &sampler) const;
    
private:
    Interaction _p0, _p1;
};

Light * createLight(const nloJson &data);

PALADIN_END

#endif /* light_hpp */
