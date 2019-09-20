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
 * 其中A(ωh)为单位宏观表面上，法线方向为ωh(法线落在微分立体角dωh内)的微面元面积之和
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
 * 不难想象，宏观表面上有若干个微平面，高度不一，微平面之间一定会有遮挡的情况
 * 
 * 所以，科学家们又定义了一个 几何遮挡函数，G(ωo, ωh)
 * 这个函数相对D函数来说就简单一些，G(ωo, ωh)∈[0,1]，其实就是遮挡的百分比
 *
 *
 *
 *
 * 
 */
class MicrofacetDistribution {
public:
    virtual ~MicrofacetDistribution() {

    }

    virtual Float D(const Vector3f &wh) const = 0;

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
     * 
     */
    Float pdfW(const Vector3f &wo, const Vector3f &wh) const {
        if (_sampleVisibleArea) {
            // 如果只计算wo视角可见部分，则需要乘以几何遮挡函数
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



PALADIN_END

#endif /* microfacet_hpp */
