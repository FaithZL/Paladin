//
//  fresnelblend.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/2/14.
//

#ifndef fresnelblend_hpp
#define fresnelblend_hpp

#include "core/bxdf.hpp"
#include "distribute.hpp"

PALADIN_BEGIN

/**
 * 菲涅尔混合
 * 用于模拟漫反射物体表面有一层光泽反射层
 *
 *                            D(ωh) F(ωo)
 * fr(p, ωo, ωi) = ------------------------------------
 *                   4(ωh · ωi) * max((n,ωi), (n,ωo))
 *
 * 根据BRDF互换性，能量守恒
 *
 * Fr(cosθ) = R +(1 - R)(1 - cosθ)^5   (这个表达式是近似)
 *
 *
 *                  28 Rd
 * fr(p, ωo, ωi) = ------- (1 - Rs) (1 - (1 - (n · ωi)/2)^5) (1 - (1 - (n · ωo)/2)^5)
 *                   23π
 *
 * 第一个表达式待推导一遍todo，后面两个还是暂时认怂吧
 *
 */
class FresnelBlend : public BxDF {
public:
    FresnelBlend(const Spectrum &Rd, const Spectrum &Rs,
                 MicrofacetDistribution *distrib)
    : BxDF(BxDFType(BSDF_REFLECTION | BSDF_GLOSSY)),
    _Rd(Rd),
    _Rs(Rs),
    _distribution(distrib) {
        
    }

    /**
     *                  28 Rd
     * fr(p, ωo, ωi) = ------- (1 - Rs) (1 - (1 - (n · ωi)/2)^5) (1 - (1 - (n · ωo)/2)^5)
     *                   23π
     * @param  wo 出射方向
     * @param  wi 入射方向
     */
    virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const;
    
    /**
     * Fr(cosθ) = R +(1 - R)(1 - cosθ)^5
     */
    Spectrum schlickFresnel(Float _cosTheta) const;
    
    /**
     * 这个函数的分布转换需要再搞搞，todo
     */
    virtual Spectrum sample_f(const Vector3f &wo, Vector3f *wi,
                              const Point2f &uOrig, Float *pdf,
                              BxDFType *sampledType) const;

    virtual Float pdfDir(const Vector3f &wo, const Vector3f &wi) const;
    
    virtual std::string toString() const;
    
private:
    const Spectrum _Rd, _Rs;
    MicrofacetDistribution *_distribution;
};

PALADIN_END

#endif /* fresnelblend_hpp */
