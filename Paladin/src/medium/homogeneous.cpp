//
//  homogeneous.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/12/21.
//

#include "homogeneous.hpp"
#include "core/sampler.hpp"
#include "core/interaction.hpp"

PALADIN_BEGIN

// Tr(p → p′) = e^(−σt * l)  volpt.hpp 中 9.1式
Spectrum HomogeneousMedium::Tr(const Ray &ray, Sampler &sampler) const {
    return Exp(-_sigma_t * std::min(ray.tMax * ray.dir.length(), MaxFloat));
}

/**
 * 采样参与介质，生成MediumInteraction对象
 *
 *
 *                   Tr(p0->p)
 *     β_surf = ------------------ 
 *                   p_surf
 *
 *
 *              σs(p+tw) Tr(p+tw->p)
 *     β_med = ----------------------   
 *                    pt(t)
 *
 * p_surf = 1 - ∫[0,t_max] pt(t) dt 
 *
 *        = 1 - ∫[0,t_max] σt e^(−σt * t) dt 
 *
 * 		  = e^(−σt * t_max)
 * 
 * 		  = Tr(p0->p)
 *
 *
 * pt(t) = σt e^(−σt * t) = σt Tr(p0->p)
 *
 */
Spectrum HomogeneousMedium::sample(const Ray &ray, Sampler &sampler, MemoryArena &arena, MediumInteraction *mi) const {
	// 随机采样一个通道
	int channel = std::min((int)(sampler.get1D() * Spectrum::nSamples),
                           Spectrum::nSamples - 1);

	//         - ln(1 - ξ)
	// dist = -------------
	//             σt
	Float dist = -std::log(1 - sampler.get1D()) / _sigma_t[channel];
	Float t = std::min(dist / ray.dir.length(), ray.tMax);
	// 是否采样到介质中
    bool sampledMedium = t < ray.tMax;
    if (sampledMedium) {
        *mi = MediumInteraction(ray.at(t), -ray.dir, ray.time, this,
                                ARENA_ALLOC(arena, HenyeyGreenstein)(_g));
    }

    // Tr(p → p′) = e^(−σt * l)
    Spectrum Tr = Exp(-_sigma_t * std::min(t, MaxFloat) * ray.dir.length());
    // pbrt里的PDF函数是取各个通道的平均值，到时候试试看直接使用当前频道的值
    // 看看会有什么结果 todo
    Spectrum density = sampledMedium ? (_sigma_t * Tr) : Tr;
    Float pdf = 0;
    for (int i = 0; i < Spectrum::nSamples; ++i) {
    	pdf += density[i];
    }
    pdf *= 1 / (Float)Spectrum::nSamples;
    if (pdf == 0) {
        CHECK(Tr.IsBlack());
        pdf = 1;
    }
    return sampledMedium ? (Tr * _sigma_s / pdf) : (Tr / pdf);
}


//"param" : {
//    "g" : 0,
//    "sigma_a" : [0.f, 0.f, 0.f],
//    "sigma_s" : [1.f, 1.f, 1.f],
//}
CObject_ptr createHomogeneousMedium(const nloJson &param, const Arguments &lst) {
    Float g = param.value("g", 0.f);
    nloJson sig_a_data = param.value("sigma_a", nloJson::array({0.f, 0.f, 0.f}));
    Spectrum sigma_a = Spectrum::FromJsonRGB(sig_a_data);
    nloJson sig_s_data = param.value("sigma_s", nloJson::array({1.f, 1.f, 1.f}));
    Spectrum sigma_s = Spectrum::FromJsonRGB(sig_s_data);
    
    auto ret = new HomogeneousMedium(sigma_a, sigma_s, g);
    return ret;
}

REGISTER("homo", createHomogeneousMedium);

PALADIN_END
