//
//  microfacet.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/9/20.
//

#ifndef distribute_hpp
#define distribute_hpp

#include "core/header.h"
#include "core/spectrum.hpp"

PALADIN_BEGIN

/**
 * 微面元分布类，包含法线分布函数跟几何遮挡函数
 *
 * 该类的一个非常重要的一个特征是法线分布函数D(ωh)
 * 大概可以理解为  D(ωh) = A(ωh) / dωh
 * 其中A(ωh)为 单位宏观表面 上，法线方向为ωh(法线落在微分立体角dωh内)的微面元面积之和
 * dωh为ωh方向的微分立体角
 *
 * 可以理解为，该函数的定义是法线方向为ωh的表面的微分面积除以ωh方向的微分立体角
 * 可以得出，一个理想的光滑平面的法线分布函数可以用一个狄拉克函数表示如下
 *               D(ωh) = δ(ωh - (0,0,1))
 *
 * 我们可以得出，法线分布函数D(ωh)的量纲为1/sr
 *
 *  ∫[hemisphere]D(ωh) cosθh dωh = 1   2式
 *
 * 2式中，D(ωh) * cosθh 表示微平面在单位宏观表面上的投影
 * 显然，半空间内积分值为1(因为各个微平面在单位宏观表面投影之和就是单位宏观表面的面积嘛)
 * 所以2式是成立的
 *
 * 从2式也可以看到D(ωh)的量纲为1/sr，没毛病
 *
 * 不难想象，宏观表面上有若干个微平面，高度不一，从光源方向，或视角方向发射的光
 * 在微平面之间可能会有遮挡，或阴影的情况
 *
 * 所以，科学家们又定义了一个 史密斯遮挡函数，G1(ωo, ωh)
 * 这个函数相对D函数来说就简单一些，G1(ωo, ωh)∈[0,1]，
 * 在讲解史密斯遮挡函数G1的定义之前
 *
 * 我们定义A+(ω)为ω方向上的正面的投影面积，A-(ω)为ω方向上背面的投影面积，则ω方向上可见面积为
 *
 *                A+(ω) - A-(ω) = area   3式
 *
 * 假设ωo为观察方向，则单位宏观表面在ωo方向上的投影面积为 area = cosθo
 *
 * 在2式中加入史密斯函数之后，得
 *
 *          area = ∫[hemisphere]G1(ωo,ωh) max(0, ωo · ωh) D(ωh)dωh
 *          因为 ωo 与 ωh 可能存在夹角，所以有一个max(0, ωo · ωh)项
 *
 *
 * 可得  area = cosθo = ∫[hemisphere]G1(ωo,ωh) max(0, ωo · ωh) D(ωh)dωh   4式
 *
 * 所以，cosθo也可表示方向 ωo 上的可见面积
 *
 * 史密斯遮挡函数的定义如下：
 *
 *              G1(ω,ωh) = (A+(ω) - A-(ω)) / A+(ω)    6式
 *
 *  用人类语言来描述，就是在ω方向上可见面积的除以在ω方向上的总面积
 *
 *
 * 接下来介绍一下几何遮挡函数G
 *
 *  定义几何遮挡函数G时，我们通常会定义一个辅助函数
 *
 *              Λ(ω) = A-(ω) / (A+(ω) - A-(ω)) = A-(ω) / cosθo    7式
 *              用来度量微平面中每个可见面积中有多少是被遮挡的面积
 *
 * 联合7式与6式，用Λ(ω)来表示G1
 *
 *          得  G1(ω) = 1 / (Λ(ω) + 1)    8式
 *
 * BRDF中的几何遮挡函数G(ωo, ωi)需要两个参数，出射方向跟入射方向
 *
 * 自然就需要两个方向的史密斯遮挡函数G1
 * 假设，两个方向的G1函数值互相独立，会有以下表达式
 *
 *   G(ωo, ωi) = G1(ωo) * G1(ωi)   9式
 *
 *  看上去好像没毛病，但经不起推敲，当ωo, ωi相等时
 *  G(ωo, ωi) = G1(ωo) 才是正确的，显然推翻了9式的猜测
 *
 *  上述情况比较特殊，更一般的来说，ωo, ωi方向越接近，则G1(ωo)与G1(ωi)的相关性越大
 *  于是，科学家们提出了一个更加精确的表达方式
 *
 *  G(ωo, ωi) = 1 / (Λ(ωo) + Λ(ωi) + 1)
 * todo，这个表达式如何推导出来的，暂时不知道，搞完主线之后再啃
 *
 */
class MicrofacetDistribution {
public:
    virtual ~MicrofacetDistribution() {

    }

    virtual Float D(const Vector3f &wh) const = 0;

    /**
     * 辅助函数Λ(ω) = A-(ω) / (A+(ω) - A-(ω))
     * @param  w [description]
     * @return   [description]
     */
    virtual Float lambda(const Vector3f &w) const = 0;

    /**
     * 史密斯遮挡函数
     * G1(ω) = 1 / (Λ(ω) + 1)
     * @param  w [description]
     * @return   [description]
     */
    Float G1(const Vector3f &w) const {
        //    if (Dot(w, wh) * CosTheta(w) < 0.) return 0.;
        return 1 / (1 + lambda(w));
    }

    /**
     * 几何遮挡函数
     * G(ωo, ωi) = 1 / (Λ(ωo) + Λ(ωi) + 1)
     * @param  w [description]
     * @return   [description]
     */
    virtual Float G(const Vector3f &wo, const Vector3f &wi) const {
        return 1 / (1 + lambda(wo) + lambda(wi));
    }

    virtual Vector3f sample_wh(const Vector3f &wo, const Point2f &u) const = 0;

    /**
     * 返回对应概率密度函数值，立体角空间
     * @param  wo 出射方向
     * @param  wh 微平面法向量
     * @return
     */
    Float pdfDir(const Vector3f &wo, const Vector3f &wh) const;

    virtual std::string toString() const = 0;

    
protected:
    
    MicrofacetDistribution(bool sampleVisibleArea)
    : _sampleVisibleArea(sampleVisibleArea) {

    }
    // 是否只采样可见区域，如果为真，则加入几何遮挡，否则忽略几何遮挡
    const bool _sampleVisibleArea;
};

/**
 * Beckmann与Spizzichino在1963年提出的一个模型
 * 传统的法线分布函数如下
 *
 *             e^(-(tanθh)^2 / α^2)
 * D(ωh) = ----------------------------
 *                π α^2 (cosθh)^4
 * σ为标准差，则 α = √2 * σ
 * 以上为各向同性的法线分布函数
 * 各项同性的意思是，θh不变，φh改变，D函数值不变
 * 各项异性θh不变，D函数值随着φh改变，而改变
 *
 * 下面来介绍一下各项异性的法线分布函数
 *
 *             e^(-(tanθh)^2 ((cosθh)^2/αx^2 + (sinθh)^2/αy^2))
 * D(ωh) = -------------------------------------------------------
 *                        π αx αy (cosθh)^4
 *
 * 可以看到，当αx与αy相等时，法线分布函数退化为各向同性
 *
 * 现在讲解一下Beckmann分布的几何遮挡函数
 *
 * 在上面的类的注释中，我们定义了一个辅助函数
 *
 *       Λ(ω) = A-(ω) / (A+(ω) - A-(ω))
 *       用来度量微平面中每个可见面积中有多少是被遮挡的面积
 *
 * Beckmann的对应Λ(ω)如下
 *
 *  Λ(ω) = 1/2 (erf(a) - 1 + e^(-a*a) / (a * √π))
 *
 * 其中 a = 1/(αtanθ)，
 *
 *                         2
 * erf(x) = ----------------------------------
 *             √π ∫[0,x] e^(- x' * x') dx'
 *
 * 我们为了避免调用std::erf() 与 std::exp()两个消耗较高的函数
 * 而采用有理多项式去逼近Λ(ω)函数
 *
 * sample_wh函数的采样方式会在函数注释中介绍
 *
 */
class BeckmannDistribution : public MicrofacetDistribution {
public:

    /**
     * 粗糙度转α参数，代码直接照搬pbrt
     * @param  roughness 粗糙度
     * @return           α值
     */
    static Float RoughnessToAlpha(Float roughness) {
        roughness = std::max(roughness, (Float)1e-3);
        Float x = std::log(roughness);
        return 1.62142f + 0.819955f * x + 0.1734f * x * x +
               0.0171201f * x * x * x + 0.000640711f * x * x * x * x;
    }

    BeckmannDistribution(Float alphax, Float alphay, bool samplevis = false)
    : MicrofacetDistribution(samplevis),
    _alphax(alphax),
    _alphay(alphay) {

    }

    /**
     * 各向异性法线分布函数
     * 当αx = αy时，为各向同性
     *             e^[-(tanθh)^2 ((cosθh)^2/αx^2 + (sinθh)^2/αy^2)]
     * D(ωh) = -------------------------------------------------------
     *                        π αx αy (cosθh)^4
     * @param  wh 微平面法线方向
     * @return    [description]
     */
    virtual Float D(const Vector3f &wh) const;

    /**
     * 采样wh向量，需要分为两种情况讨论，各向异性跟各向同性
     * 先来介绍一下忽略遮挡的全采样
     * 基本思路就是根据法线分布函数分解为θ与φ的分布
     * 分别独立采样这两个一维变量，然后再计算θ与φ对应的方向向量
     *
     * θ与φ相互独立，p(θ,φ) = pθ(θ) * pφ(φ)   1式
     *
     * pφ为均匀分布，pφ = 1 / 2π
     * 假设 a为[0，1)均匀随机变量，则φ = 2πa
     *
     * pθ(θ)就比较复杂了
     * sampling.hpp中4式  p(θ, φ) = sinθ p(ω)   (在这里叫做2式)
     *
     * pω(ωh) = cosθh * D(ωh)   (显而易见，我就不废话了)  3式
     *
     * 以上2，3两式联合可得
     *
     * sinθh * cosθh * D(ωh) = p(θ, φ)    4式
     *
     * 1，4两式联合可得，
     *
     * pθ(θ) = sinθ * cosθ * D(ω) / pφ(φ)   5式
     *
     * 将pφ = 1 / 2π带入5式，得
     *
     * pθ(θ) = 2π * sinθ * cosθ * D(ω)   6式
     *
     * 将已知D(ω)带入6式，得
     *             2sinθ * e^(-(tanθ)^2 / α^2)
     * pθ(θ) = -----------------------------------    8式 (我知道这跟pbrbook上的表达式不同，那是pbrbook上写错了)
     *                    α^2 (cosθ)^3
     *
     * 现在已知pdf，要求cdf，对pθ(θ)积分，手算实在是太麻烦(主要是不会😂)，交给wolframalpha吧，可得
     *
     * Pθ(θ) = 1 - e^(-(tanθ)^2 / α^2)  9式
     *
     * 知道9式之后我们可以用均匀随机变量生成θ了  (逆变换算法不解释！)
     *
     * 假设 b为[0，1)均匀随机变量
     *
     * b = 1 - e^(-(tanθ)^2 / α^2)
     *
     * 则 (tanθ)^2 = -α^2 ln(1-b)
     *
     * 又因为b为[0，1)均匀随机变量，可得 (tanθ)^2 = -α^2 ln(b)
     *
     * 生成θ与φ之后再转成向量就完事了
     *
     * 各向异性的采样函数也可以通过类似的方法推导
     * 总结来就是以下几个步骤
     *
     * 1.把立体角空间的概率密度函数分解为θ与φ两个独立的概率密度函数(以下简称pdf)
     * 2.分别对θ与φ的pdf积分得到对应的累积分布函数(cdf)
     * 3.用逆变换算法对cdf的反函数在[0,1)区间进行采样
     * 4.用生成的θ与φ计算向量
     * 5.完事！
     *
     * GGXDistribution::sample_wh的采样表达式也可以用上述方式推导出来
     * 在这里就不再赘述了
     *
     */
    virtual Vector3f sample_wh(const Vector3f &wo, const Point2f &u) const;

    virtual std::string toString() const {
        return StringPrintf("[ BeckmannDistribution alphax: %f alphay: %f ]",
                            _alphax, _alphay);
    }

private:

    /**
     * 原始表达式如下
     *  Λ(ω) = 1/2 (erf(a) - 1 + e^(-a*a) / (a * √π))
     *
     * 其中 a = 1/(αtanθ)，
     *                         2
     * erf(x) = ----------------------------------
     *             √π ∫[0,x] e^(- x' * x') dx'
     * 为了避免调用std::erf() 与 std::exp()两个消耗较高的函数
     * 我们采用有理多项式去逼近
     * 至于如何去逼近的，暂时没有搞懂推导，todo欠着，啃完主线再来
     * @param  w
     * @return
     */
    virtual Float lambda(const Vector3f &w) const;

    const Float _alphax, _alphay;
};

/**
 * Trowbridge 与 Reitz在 1975年提出的微平面模型
 * 又由Walter在2007年的时候独立推导出来了，又叫GGX模型
 * 各向异性的法线分布函数为
 *                                             1
 * D(ωh) = ----------------------------------------------------------------------------
 *             π αx αy (cosθh)^4 [1 + (tanθh)^2 ((cosθh)^2/αx^2 + (sinθh)^2/αy^2)]^2
 *
 * Trowbridge模型与Beckmann的模型相比有什么不同？
 * http://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models.html
 * 中有图例介绍
 * 大概就是Trowbridge图像在两端的拖尾比较长，在接近π/2的时候还明显大于零
 * 而Beckmann在π/4就很接近0了  (两个分布都取α=0.5)
 *
 * 至于这个函数是如何推导的，我也就不凑热闹了，能用好就不错了
 *
 * 现在来介绍一下Trowbridge的辅助函数Λ(ω)，相比Beckmann的辅助函数，TrowbridgeReitz就简单很多了
 *
 * Λ(ω) = [-1 + √(1 + (α tanθ)^2)] / 2
 *
 */
class GGXDistribution : public MicrofacetDistribution {
public:
    /**
     * 粗糙度转α参数，代码直接照搬pbrt
     * @param  roughness 粗糙度
     * @return           α值
     */
    static inline Float RoughnessToAlpha(Float roughness) {
        roughness = std::max(roughness, (Float)1e-3);
        Float x = std::log(roughness);
        return 1.62142f + 0.819955f * x + 0.1734f * x * x + 0.0171201f * x * x * x +
        0.000640711f * x * x * x * x;
    }
    
    GGXDistribution(Float alphax, Float alphay,
                                bool samplevis = false)
    : MicrofacetDistribution(samplevis),
    _alphax(alphax),
    _alphay(alphay) {
        
    }
    
    /**
     * 法线分布函数
     *                                             1
     * D(ωh) = -----------------------------------------------------------------------------
     *             π αx αy (cosθh)^4 [1 + (tanθh)^2 ((cosθh)^2/αx^2 + (sinθh)^2/αy^2)]^2
     *
     * @param  wh 微平面法向量
     * @return    [description]
     */
    virtual Float D(const Vector3f &wh) const;
    
    virtual Vector3f sample_wh(const Vector3f &wo, const Point2f &u) const;
    
    virtual std::string toString() const {
        return StringPrintf("[ GGXDistribution alphax: %f alphay: %f ]",
                            _alphax, _alphay);
    }
    
private:

    /**
     * Λ(ω) = [-1 + √(1 + (α tanθ)^2)] / 2
     *
     * @param  w [description]
     * @return   [description]
     */
    virtual Float lambda(const Vector3f &w) const;
    
    // todo 这里也是可以优化的
    const Float _alphax, _alphay;
};

PALADIN_END

#endif /* distribute_hpp */
