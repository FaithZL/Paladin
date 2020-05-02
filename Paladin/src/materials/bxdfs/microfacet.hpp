//
//  microfacet.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/4/28.
//

#ifndef microfacet_hpp
#define microfacet_hpp

#include "core/header.h"
#include "core/spectrum.hpp"
#include "math/frame.hpp"

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
class Microfacet {
public:
    enum EType {
        GGX,
        Beckmann
    };
    
    inline Microfacet(Float alpha, EType type = GGX,bool sampleVisibleArea = false)
    :_type(type),
    _alphaU(alpha),
    _alphaV(alpha),
    _sampleVisibleArea(sampleVisibleArea) {
        _alphaU = std::max(_alphaU, (Float) 1e-4f);
        _alphaV = std::max(_alphaV, (Float) 1e-4f);
    }
    
    inline Microfacet(Float alphaU, Float alphaV, EType type = GGX,bool sampleVisibleArea = false)
    :_type(type),
    _alphaU(alphaU),
    _alphaV(alphaV),
    _sampleVisibleArea(sampleVisibleArea) {
        _alphaU = std::max(_alphaU, (Float) 1e-4f);
        _alphaV = std::max(_alphaV, (Float) 1e-4f);
    }
    
    ~Microfacet() {

    }

    Float D(const Vector3f &wh) const;

    /**
     * 辅助函数Λ(ω) = A-(ω) / (A+(ω) - A-(ω))
     * @param  w [description]
     * @return   [description]
     */
    Float lambda(const Vector3f &w) const;

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
    Float G(const Vector3f &wo, const Vector3f &wi) const {
        return 1 / (1 + lambda(wo) + lambda(wi));
    }

    Vector3f sample_wh(const Vector3f &wo, const Point2f &u) const;

    /**
     * 返回对应概率密度函数值，立体角空间
     * @param  wo 出射方向
     * @param  wh 微平面法向量
     * @return
     */
    Float pdfDir(const Vector3f &wo, const Vector3f &wh) const;

    std::string toString();
    
    inline EType getType() const {
        return _type;
    }
    
    inline Float getAlphaU() const {
        return _alphaU;
    }
    
    inline Float getAlphaV() const {
        return _alphaV;
    }
    
    inline Float getAlpha() const {
        return _alphaU;
    }
    
    inline bool isIsotropic() const {
        return _alphaU == _alphaV;
    }
    
protected:
    
    EType _type;
    Float _alphaU, _alphaV;
    
    // 是否只采样可见区域，如果为真，则加入几何遮挡，否则忽略几何遮挡
    const bool _sampleVisibleArea;
};



PALADIN_END

#endif /* microfacet_hpp */
