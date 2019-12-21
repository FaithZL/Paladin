//
//  homogeneous.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/12/21.
//

#include "homogeneous.hpp"

PALADIN_BEGIN

// Tr(p → p′) = e^(−σt * l)  volpt.hpp 中 9.1式
Spectrum HomogeneousMedium::Tr(const Ray &ray, Sampler &sampler) const {
    return Exp(-_sigma_t * std::min(ray.tMax * ray.dir.length(), MaxFloat));
}

PALADIN_END
