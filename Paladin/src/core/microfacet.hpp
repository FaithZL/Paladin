//
//  microfacet.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/9/20.
//

#ifndef microfacet_hpp
#define microfacet_hpp

#include "header.h"
#include "bxdf.hpp"

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
 * 所以，科学家们又定义了一个 几何遮挡函数，G1(ωo, ωh)
 * 这个函数相对D函数来说就简单一些，G1(ωo, ωh)∈[0,1]，其实就是遮挡的百分比
 * 
 * 假设ωo为观察方向，则单位宏观表面在ωo方向上的投影面积为 area = cosθo
 * 加入几何遮挡函数之后 area = ∫[hemisphere]G1(ωo,ωh) max(0, ωo · ωh) D(ωh)dωh
 * 因为 ωo 与 ωh 可能存在夹角，所以有一个max(0, ωo · ωh)项
 *
 * 可得       cosθo = ∫[hemisphere]G1(ωo,ωh) max(0, ωo · ωh) D(ωh)dωh   3式
 * 
 * cosθo也可表示方向 ωo 上的可见面积
 * 
 * 我们定义A+(ω)为ω方向上的正面的投影面积，A-(ω)为ω方向上背面的投影面积，则ω方向上可见面积为 
 * 
 *             A+(ω) - A-(ω) = cosθo
 *             
 * 则ω方向上可见面积的比例为：
 *
 *              G1(ω,ωh) = (A+(ω) - A-(ω)) / A+(ω)
 *
 *  定义遮挡函数，我们通常会定义一个辅助函数 
 *  
 *              Λ(ω) = A-(ω) / (A+(ω) - A-(ω)) = cosθo
 *              用来度量微平面中每个可见面积中有多少是被遮挡的面积
 *              
 * 
 */
class MicrofacetDistribution {
public:
    virtual ~MicrofacetDistribution() {

    }

    virtual Float D(const Vector3f &wh) const = 0;

    /**
     * 辅助函数 G1(ω,ωh) = (A+(ω) - A-(ω)) / A+(ω)
     * @param  w [description]
     * @return   [description]
     */
    virtual Float lambda(const Vector3f &w) const = 0;

    Float G1(const Vector3f &w) const {
        //    if (Dot(w, wh) * CosTheta(w) < 0.) return 0.;
        return 1 / (1 + lambda(w));
    }

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
    Float pdfW(const Vector3f &wo, const Vector3f &wh) const {
        if (_sampleVisibleArea) {
            // 如果只计算wo视角可见部分，则需要乘以几何遮挡函数再归一化
            // 归一化方式如下，利用以上3式
            // cosθo = ∫[hemisphere]G1(ωo,ωh) max(0, ωo · ωh) D(ωh)dωh
            // 将cos项移到右边，得
            // 1 = ∫[hemisphere]G1(ωo,ωh) max(0, ωo · ωh) D(ωh) / (cosθo) dωh
            // 则概率密度函数为 G1(ωo,ωh) max(0, ωo · ωh) D(ωh) / (cosθo) ,代码如下
            return D(wh) * G1(wo) * absDot(wo, wh) / absCosTheta(wo);
        } else {
            // 如果忽略几何遮挡，则概率密度函数值就是D(ωh) * cosθh 
            return D(wh) * absCosTheta(wh);
        }
    }

    virtual std::string toString() const = 0;

    
protected:
    
    MicrofacetDistribution(bool sampleVisibleArea)
    : _sampleVisibleArea(sampleVisibleArea) {

    }
    // 是否只采样可见区域，如果为真，则加入几何遮挡，否则忽略几何遮挡
    const bool _sampleVisibleArea;
};


class BeckmannDistribution : public MicrofacetDistribution {
public:
    static Float RoughnessToAlpha(Float roughness) {
        roughness = std::max(roughness, (Float)1e-3);
        Float x = std::log(roughness);
        return 1.62142f + 0.819955f * x + 0.1734f * x * x +
               0.0171201f * x * x * x + 0.000640711f * x * x * x * x;
    }

    BeckmannDistribution(Float alphax, Float alphay, bool samplevis = true)
    : MicrofacetDistribution(samplevis), 
    _alphax(alphax), 
    _alphay(alphay) {

    }

    virtual Float D(const Vector3f &wh) const {
        Float _tan2Theta = tan2Theta(wh);
        if (std::isinf(_tan2Theta)) {
            return 0.;
        }
        Float cos4Theta = cos2Theta(wh) * cos2Theta(wh);
        Float ret = std::exp(-_tan2Theta * (cos2Phi(wh) / (_alphax * _alphax) +
                                      sin2Phi(wh) / (_alphay * _alphay))) /
               (Pi * _alphax * _alphay * cos4Theta);
        return ret;
    }

    virtual Vector3f sample_wh(const Vector3f &wo, const Point2f &u) const;

    virtual std::string toString() const;

private:

    Float lambda(const Vector3f &w) const {
        Float absTanTheta = std::abs(tanTheta(w));
        if (std::isinf(absTanTheta)) {
            return 0.;
        }
        Float alpha = std::sqrt(cos2Phi(w) * _alphax * _alphax + sin2Phi(w) * _alphay * _alphay);
        Float a = 1 / (alpha * absTanTheta);
        if (a >= 1.6f) {
            return 0;
        }
        return (1 - 1.259f * a + 0.396f * a * a) / (3.535f * a + 2.181f * a * a);
    }

    const Float _alphax, _alphay;
};

PALADIN_END

#endif /* microfacet_hpp */
