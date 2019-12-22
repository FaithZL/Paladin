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
	int channel = std::min((int)(sampler.get1D() * Spectrum::nSamples),
                           Spectrum::nSamples - 1);
}


PALADIN_END
