//
//  pathtracer.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef pathtracer_hpp
#define pathtracer_hpp

#include "core/integrator.hpp"
#include "math/lightdistribute.hpp"

PALADIN_BEGIN



class PathTracer : public MonteCarloIntegrator {
public:
	PathTracer(int maxDepth, std::shared_ptr<const Camera> camera,
                   std::shared_ptr<Sampler> sampler,
                   const AABB2i &pixelBounds, Float rrThreshold = 1,
                   const std::string &lightSampleStrategy = "Power");

	virtual void preprocess(const Scene &scene, Sampler &sampler);
	
	virtual Spectrum Li(const RayDifferential &ray, const Scene &scene,
                Sampler &sampler, MemoryArena &arena, int depth) const;

private:
	// 最大反射次数
	const int _maxDepth;
	// 俄罗斯轮盘结束的阈值
    const Float _rrThreshold;
    // 官员采样策略
    const std::string _lightSampleStrategy;
};



PALADIN_END

#endif /* pathtracer_hpp */
