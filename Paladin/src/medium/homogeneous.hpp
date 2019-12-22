//
//  homogeneous.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/12/21.
//

#ifndef homogeneous_hpp
#define homogeneous_hpp

#include "core/medium.hpp"
#include "core/spectrum.hpp"

PALADIN_BEGIN


/**
 * Ls(p,w) = Le(p,w) + σs(p,w) ∫[sphere] p(p,w,wi) Li(p,wi) dwi
 * 通常，介质不会发光，所以上式可以简化为
 *
 * Ls(p,w) = σs(p,w) ∫[sphere] p(p,w,wi) Li(p,wi) dwi
 * 
 * Li(p,w) = Tr(p0->p) Lo(p0,-w) + ∫[0,t_max] Tr(p+tw->p) Ls(p+tw,-w) dt
 * 
 *
 * 
 */
class HomogeneousMedium : public Medium {
    
public:
    HomogeneousMedium(const Spectrum &sigma_s, const Spectrum &sigma_a, Float g)
    :_sigma_s(sigma_s),
    _sigma_a(sigma_a),
    _sigma_t(sigma_a + sigma_s),
    _g(g) {

    }

	virtual Spectrum Tr(const Ray &ray, Sampler &sampler) const override;

	virtual Spectrum sample(const Ray &ray, Sampler &sampler, MemoryArena &arena,
						MediumInteraction *mi) const override;

private:
	// 散射系数
	const Spectrum _sigma_s;
	// 吸收系数
	const Spectrum _sigma_a;
	// 传播系数
	const Spectrum _sigma_t;
	// 各向异性系数
	const Float _g;
};

PALADIN_END

#endif /* homogeneous_hpp */
