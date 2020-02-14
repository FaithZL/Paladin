//
//  transmission.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/2/14.
//

#ifndef transmission_hpp
#define transmission_hpp

#include "distribute.hpp"
#include "core/bxdf.hpp"

PALADIN_BEGIN

/**
 * 微面元透射 BTDF MicrofacetTransmission
 * 我们在实现 MicrofacetReflection类的时候已经推导过dωo与dωh的关系式
 *           dωo
 * dωh = ----------
 *         4cosθh
 * 但在 MicrofacetTransmission中，这两者的关系式是不同的
 * 关系式如下
 *                 ηo^2 |ωo · ωh| dωo
 *    dωh = --------------------------------            1式   (这个没有手动推过，比较羞耻，todo，有空一定要搞搞)
 *            [ηi(ωh · ωi) + ηo(ωo · ωh)]^2
 *
 * 补上1式推导的思路，我们需要求的是dωh/dωo，实际上可以把ωi指定为基准向量(θ为零)，ωi向量方向不变
 * 我们可以计算ωh向量变化时，对应ωo的变化率，实际上就是dωh/dωo的倒数，则
 * θi为入射角，θo为折射角
 * 又因为ωi指定为基准向量，ωh与ωi的夹角 = π + θi - θo，ωo与ωi的夹角 = π - θo
 * 由立体角定义展开如下
 *
 *              sin(π + θi - θo)d(π + θi - θo)dφh
 * dωh/dωo = ------------------------------------    (φh = φo，可以约分)
 *                   sin(π - θo)d(π - θo)dφo
 *
 * 上述有两个未知量θi，θo，又由斯涅尔定律  ηi * sinθi = ηo * sinθo
 * 用斯涅尔定律表达式带入dωh/dωo表达式，各种换元法，求导。。。。。。应该就可以推导出1式了
 * (计算量太大了，暂未尝试，先欠着)
 *
 *
 *
 *                 (1 - Fr(ωo)) Li(ωi) dωi D(ωh) dωh dA cosθh
 *    Lo(ωo) = ------------------------------------------------   2式 (MicrofacetReflection中的7式修改而来)
 *                          cosθo dωo dA
 *
 *                             dLo(p, ωo)               dLo(p, ωo)
 * 由BTDF定义 fr(p, ωo, ωi) = ------------------ = ------------------------   3式(MicrofacetReflection中的10式)
 *                              dE(p, ωi)           Li(p, ωi) cosθi dωi
 *
 * 联合1，2，3式，再加入几何遮挡函数
 *
 *                 η^2 (1 - Fr(ωo)) D(ωh) G(ωi,ωo)   |ωi · ωh||ωo · ωh|
 * fr(ωo, ωi) = ---------------------------------------------------------
 *                   [(ωh · ωo) + η(ωi · ωh)]^2   cosθo cosθi
 *
 * 其中 η = ηi/ηo , ωh = ωo + ηωi
 *
 */
class MicrofacetTransmission : public BxDF {
public:
    MicrofacetTransmission(const Spectrum &T,
                           MicrofacetDistribution *distribution, Float etaA,
                           Float etaB, TransportMode mode)
    : BxDF(BxDFType(BSDF_TRANSMISSION | BSDF_GLOSSY)),
    _T(T),
    _distribution(distribution),
    _etaA(etaA),
    _etaB(etaB),
    _fresnel(etaA, etaB),
    _mode(mode) {
        
    }
    
    /**
     * BTDF 函数值
     *                η^2 (1 - Fr(ωo)) D(ωh) G(ωi,ωo)   |ωi · ωh||ωo · ωh|
     * fr(ωo, ωi) = ---------------------------------------------------------
     *                   [(ωh · ωo) + η(ωi · ωh)]^2    cosθo cosθi
     * @param  wo 出射向量
     * @param  wi 入射向量
     * @return    BTDF函数值
     */
    virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const;
    
    virtual Spectrum sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                              Float *pdf, BxDFType *sampledType) const;
    
    /**
     * dωh 与 dωi的关系式
     *                   ηi^2 |ωi · ωh|
     * dωh/dωi = --------------------------------    (推导思路与dωh/dωo类似)
     *            [ηo(ωh · ωo) + ηi(ωi · ωh)]^2
     *
     *  又由sampling.hpp 中 1 式 py(y) * dy/dx = px(x) 可得
     *
     * pωi(ωi) = dωh / dωi * pωh(ωh)
     *
     * @param  wo [description]
     * @param  wi [description]
     * @return    [description]
     */
    virtual Float pdfDir(const Vector3f &wo, const Vector3f &wi) const;
    
    virtual std::string toString() const;
    
private:

    const Spectrum _T;
    const MicrofacetDistribution *_distribution;
    const Float _etaA, _etaB;
    const FresnelDielectric _fresnel;
    const TransportMode _mode;
};

PALADIN_END

#endif /* transmission_hpp */
