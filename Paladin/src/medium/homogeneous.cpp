//
//  homogeneous.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/12/21.
//

#include "homogeneous.hpp"
#include "core/sampler.hpp"

PALADIN_BEGIN

// Tr(p → p′) = e^(−σt * l)  volpt.hpp 中 9.1式
Spectrum HomogeneousMedium::Tr(const Ray &ray, Sampler &sampler) const {
    return Exp(-_sigma_t * std::min(ray.tMax * ray.dir.length(), MaxFloat));
}

// 采样参与介质，生成MediumInteraction对象
Spectrum HomogeneousMedium::sample(const Ray &ray, Sampler &sampler, MemoryArena &arena, MediumInteraction *mi) const {
	// 随机采样一个通道
	int channel = std::min((int)(sampler.get1D() * Spectrum::nSamples),
                           Spectrum::nSamples - 1);

	//         - ln(1 - ξ)
	// dist = -------------
	//             σt
	Float dist = -std::log(1 - sampler.get1D()) / _sigma_t[channel];
	Float t = std::min(dist / ray.d.Length(), ray.tMax);
    bool sampledMedium = t < ray.tMax;
    if (sampledMedium)
        *mi = MediumInteraction(ray(t), -ray.d, ray.time, this,
                                ARENA_ALLOC(arena, HenyeyGreenstein)(g));
}


PALADIN_END
