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
 * 微面元分布类
 * 该类的一个非常重要的一个特征是法线分布函数D(wh)
 * 该函数描述的是法线方向为wh的表面的微分面积
 * 一个理想的光滑平面的法线分布函数可以用一个狄拉克函数表示如下
 * D(wh) = δ(wh - (0,0,1))
 * 
 * 法线分布函数必须归一化，保证物理上的真实性
 * 
 * ∫[hemisphere]D(wh) cosθh dθh = 1
 *
 *
 * 
 */
class MicrofacetDistribution {
public:
    virtual ~MicrofacetDistribution();

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

    Float pdfW(const Vector3f &wo, const Vector3f &wh) const {
        if (_sampleVisibleArea) {
            return D(wh) * G1(wo) * absDot(wo, wh) / absCosTheta(wo);
        } else {
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
