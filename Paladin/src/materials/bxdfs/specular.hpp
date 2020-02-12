//
//  specular.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/2/12.
//

#ifndef specular_hpp
#define specular_hpp

#include "core/bxdf.hpp"

PALADIN_BEGIN

/**
 * 理想镜面反射
 * 狄拉克delta分布
 */
class SpecularReflection : public BxDF {
    
public:
    SpecularReflection(const Spectrum &R, Fresnel *fresnel)
    : BxDF(BxDFType(BSDF_REFLECTION | BSDF_SPECULAR)),
    _R(R),
    _fresnel(fresnel) {
        
    }
    
    /**
     * 由于是理想镜面反射，狄拉克函数
     * 用常规方式无法采样，需要特殊处理
     * @param  wo 出射方向
     * @param  wi 入射方向
     * @return    0
     */
    virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const {
        return Spectrum(0.f);
    }
    
    /**
     * 理想镜面反射的采样函数，是固定方向采样，首先要计算合适的入射方向
     * Lo的蒙特卡洛估计表达式如下
     *             1              fr(p,wo,wi)Li(p,wi)|cosθi|
     * Lo(wo) = ----- * Σ[i,N] ------------------------------
     *             N                       p(wi)
     *
     *             1              (ρhd(wo)δ(wi-wr)/|cosθi|)Li(p,wi)|cosθi|
     * Lo(wo) = ----- * Σ[i,N] --------------------------------------------
     *             N                            p(wi)
     *
     * p(wi)为狄拉克函数，p(wi) = δ(wi-wr)，两个狄拉克函数可以抵消，
     *
     * 最终得到Lo(wo) = ρhd(wo) * Li(p, wr)
     * 由于是非常规采样，pdf为1
     *
     *                 δ(wi - wr)Fr(wi)
     * fr(p,wo,wi) = -----------------------   3式
     *                     |cosθr|
     *
     * ρhd(wo) = ∫[hemisphere]fr(p,wi,wo)|cosθi|dwi 4式 (ρhd(wo)的定义)
     *
     * 联合3,4式可得如下代码
     *
     * @param  wo          出射方向
     * @param  wi          入射方向
     * @param  sample      在此函数中不需要使用的样本点
     * @param  pdf         概率密度函数值为1
     * @param  sampledType 不需要使用的采样类型
     * @return             [description]
     */
    virtual Spectrum sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample,
                              Float *pdf, BxDFType *sampledType) const ;
    /**
     * 由于是理想镜面反射，狄拉克函数
     * 用常规方式无法采样，需要特殊处理
     * @param  wo 出射方向
     * @param  wi 入射方向
     * @return    0
     */
    virtual Float pdfDir(const Vector3f &wo, const Vector3f &wi) const {
        return 0;
    }
    
    virtual std::string toString() const;
    
private:
    // 用于缩放颜色频谱
    const Spectrum _R;
    // 菲涅尔对象指针
    const Fresnel *_fresnel;
};

/**
 * 理想镜面透射
 * 用于建立光线照射在水面上的数学模型
 *
 * 先来简单推导一下所用到的表达式
 * 想象一下光线照射在水面上，一部分光直接反射，另一部分光射入水中，
 * 反射光的radiance我们可以直接通过菲涅尔函数计算出来
 * 折射光的radiance就没那么简单，因为在两个介质交界处，光线的微分立体角会发生变化(因为折射率不同)
 *
 * 根据能量守恒定律，微分立体角发生变化，radiance也就发生了变化
 * 我们用τ表示被折射的能量的比率则τ = 1 - Fr(wi)
 * Φ 表示辐射通量
 *
 * dΦo = τ * dΦi
 * 将radiance带入，得
 *
 * Lo * cosθo * dA * dωo = τ(Li * cosθi * dA * dωi)
 *
 * 将微分立体角展开，得
 *
 * Lo * cosθo * dA * sinθodθodφo = τ * Li * cosθi * dA * sinθidθidφi   2式
 *
 * 对斯涅尔定律ηi * sinθi = ηo * sinθo 求导，得
 *
 * ηo * cosθodθo = ηi * cosθidθi  3式
 *
 * 联合2，3式，得
 *
 * Lo * ηi^2 * dφo = τLi * ηo^2 * dφi
 *
 * 整理之后，如下
 *
 * Lo = τ * Li * (ηo/ηi)^2
 *
 * BTDF的表达式如下
 *                ηo                          δ(wi - T(wo,n))
 * fr(wo, wi) = (----)^2 * (1 - Fr(wi)) * ----------------------     (T为折射函数，n为法线)
 *                ηi                            |cosθi|
 *
 */
class SpecularTransmission : public BxDF {
public:
    SpecularTransmission(const Spectrum &T, Float etaA, Float etaB,
                         TransportMode mode)
    : BxDF(BxDFType(BSDF_TRANSMISSION | BSDF_SPECULAR)),
    _T(T),
    _etaA(etaA),
    _etaB(etaB),
    _fresnel(etaA, etaB),
    _mode(mode) {
        
    }
    
    virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const {
        return Spectrum(0.f);
    }
    
    /**
     * 采样理想镜面透射
     * BTDF的表达式如下
     *                ηo                          δ(wi - T(wo,n))
     * fr(wo, wi) = (----)^2 * (1 - Fr(wi)) * ----------------------
     *                ηi                            |cosθi|
     *
     * 跟镜面反射一样，无法用常规方式采样处理
     * pdf直接赋值为1，原理与上面的理想镜面反射相同，不再赘述
     *
     */
    virtual Spectrum sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample,
                              Float *pdf, BxDFType *sampledType) const;
    
    virtual Float pdfDir(const Vector3f &wo, const Vector3f &wi) const {
        return 0;
    }
    
    virtual std::string toString() const;
    
private:
    // 用于缩放颜色频谱
    const Spectrum _T;

    // _etaA是表面以上介质的折射率 (above)
    // _etaB是物体表面以下介质的折射率 (below)
    const Float _etaA, _etaB;
    const FresnelDielectric _fresnel;
    // 记录传输模式(从光源发出or从相机发出)
    const TransportMode _mode;
};


PALADIN_END

#endif /* specular_hpp */
