//
//  reflection.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/2/14.
//

#ifndef reflection_hpp
#define reflection_hpp

#include "core/bxdf.hpp"
#include "distribute.hpp"

PALADIN_BEGIN

/**
 *
 * 微面元反射DRDF
 * 关于微面元的法线分布函数介绍在microfacet.hpp文件中
 * 一个较早的微平面模型在1967年 Torrance 和 Sparrow用于模拟金属表面
 * 假设一个宏观平面有若干个微平面构成，微平面的法向量为ωh
 * ωh与ωo的夹角为θh   ωh与ωi夹角为θh
 * 观察光线出射方向为ωo，入射方向为ωi则满足，
 *
 *     ωh = ωo + ωi   1式
 *
 * 现在我们来推导一下微面元反射的BRDF表达式
 *
 * 先来看看h方向上的微分辐射通量的表达式，根据flux与radiance的关系，可得
 *
 *      dΦh = Li(ωi) dω dA⊥(ωh) = Li(ωi) dω cosθh dA(ωh)    2式
 *
 * ωh方向上的微平面面积如下
 *
 *      dA(ωh) = D(ωh) dωh dA       3式      (由法线分布函数的定义可得)
 *
 * 将3式带入2式
 *
 *      dΦh = Li(ωi) dω cosθh D(ωh) dωh dA     4式
 *
 * 我们假设各个微平面根据菲涅尔定律独立反射光线，则出射辐射通量flux为
 *
 *      dΦo = Fr(ωo) dΦh   5式    (这里理解得不是很好，菲涅尔函数表示的就是有多少能量没被吸收，直接反射出来了)
 *
 * 再由radiance的定义
 *                     dΦo
 *      Lo(ωo) = ------------------     6式
 *                 cosθo dωo dA
 *
 * 联合4，5，6式，可得
 *
 *                  Fr(ωo) Li(ωi) dωi D(ωh) dωh dA cosθh
 *      Lo(ωo) = ---------------------------------------------          7式
 *                              cosθo dωo dA
 *
 * 这里直接使用一个结论表达式(在pdfDir方法的注释中给出推导过程)
 *
 *      dωo = 4 cosθh dωh       8式
 *
 * 联合7，8式
 *                 Fr(ωo) Li(ωi) D(ωh) dωi
 *      Lo(ωo) = -----------------------------           9式
 *                       4 cosθo
 *
 *                             dLo(p, ωo)               dLo(p, ωo)
 * 由BRDF定义 fr(p, ωo, ωi) = ------------------ = ------------------------   10式
 *                              dE(p, ωi)           Li(p, ωi) cosθi dωi
 *
 * 10式定义了fr，用人话来说就是fr(p, ωo, ωi)表示的是：
 * ωi方向辐照度(irradiance)变化时，ωo方向的辐射度(radiance)的变化率
 *
 * 联合9，10两式，再加入几何遮挡项
 *
 *                   D(ωh) G(ωi,ωo) Fr(ωo)
 * fr(p, ωo, ωi) = ------------------------     11式
 *                     4 cosθo cosθi
 *
 * 到此，BRDF就这样推导完毕
 *
 * 顺便说一下，为何BRDF的定义为辐射度比辐照度，量纲为1/sr（sr为立体角）
 * 说一下我的个人理解
 *
 * 我们观察到一个物体的表面的点，被无数个方向的光照射到，然后向我们眼睛的方向反射了一部分光
 * 所以我们才能看到该点
 *
 * 我们需要定义一个分布函数，用来表示每个入射方向的光线的贡献大小
 *
 * Lo(wo) = ∫[hemisphere]fr(ωo,ωi)Li(ωi)|cosθi|dωi      12式
 *
 * 12式表达了半空间上所有方向的入射辐射度乘以fr之后再相加，得到的是出射辐射度
 *
 * 由以上表达式，自然可以得出一个结论fr的量纲为1/sr（sr为立体角）
 * 感觉自己说得也不清晰
 *
 */
class MicrofacetReflection : public BxDF {
public:
    MicrofacetReflection(const Spectrum &R,
                         MicrofacetDistribution *distribution, Fresnel *fresnel)
    : BxDF(BxDFType(BSDF_REFLECTION | BSDF_GLOSSY)),
    _R(R),
    _distribution(distribution),
    _fresnel(fresnel) {
        
    }
    
    /**
     * 函数值，原始表达式如下
     *                   D(ωh) G(ωi,ωo) Fr(ωo)
     * fr(p, ωo, ωi) = ------------------------
     *                     4 cosθo cosθi
     * @param  wo 出射方向
     * @param  wi 入射方向
     * @return    [description]
     */
    Spectrum f(const Vector3f &wo, const Vector3f &wi) const;
    
    /**
     * 函数的样本值，并返回该样本的概率密度函数值
     * @param  wo          出射方向
     * @param  wi          输出的入射方向
     * @param  u           决定入射方向的样本点
     * @param  pdf         输出的概率密度函数值
     * @param  sampledType 采样类型
     * @return             [description]
     */
    Spectrum sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                      Float *pdf, BxDFType *sampledType) const;
    
    /**
     * 采样wi概率密度函数，
     * 在MicrofacetDistribution中我们只实现了wh的概率密度函数
     * 这里要将wh的分布转换到wi的分布
     * θi为ωi与ωo的夹角，θh为ωh与ωo的夹角，ωo方向不变，ωh的改变会引起ωi的变换，
     * 知道以上信息之后，可以开始推导了
     *
     *      θi = 2θh,  dφi = dφh
     *
     *                 sinθh dθh dφh
     * dωh / dωi = --------------------
     *                 sinθi dθi dφi
     *
     *                 sinθh dθh            sinθh           1
     * dωh / dωi = -------------------- = ---------- = ---------- = dωh / dωo
     *                sin2θh d2θh          2sin2θh       4cosθh
     *
     * 又由sampling.hpp 中 1 式 py(y) * dy/dx = px(x) 可得
     *
     * pωi(ωi) = dωh / dωi * pωh(ωh) = pωh(ωh) / 4cosθh
     *
     * @param  wo 出射方向
     * @param  wi 入射方向
     * @return    [description]
     */
    Float pdfDir(const Vector3f &wo, const Vector3f &wi) const;
    
    std::string toString() const;
    
private:
    // 反射率
    const Spectrum _R;
    // 微平面分布
    const MicrofacetDistribution *_distribution;
    // 菲涅尔
    const Fresnel *_fresnel;
};

PALADIN_END

#endif /* reflection_hpp */
