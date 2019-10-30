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

/**
 * 接下来要介绍一种全局光照积分器，也是工业界应用最广泛的积分器
 * 路径追踪
 * 
 * 之前说了，渲染的问题本质上都是积分的问题，渲染的本质就是求解渲染方程，或光照传输方程(LTE)
 * LTE的形式如下
 * 
 * Lo(p, ωo) = Le(p, ωo) + c ∫[hemisphere]Lo(t(p, ωi), -ωi)|cosθi|dωi
 * 
 * 可以看到，目前LTE是对方向的积分，因为顺着方向一定可以找到空间中的一个点
 * 我们可以把对方向的积分转化为对顶点的积分，方向积分之后就是立体角，点积分之后就是面积
 * (说实话我表达得很不好，哎，水平太低，但是看了表达式之后应该就懂了！！！)
 * 
 * 我们定义光线离开点p'到p的辐射度为 
 *
 * 		L(p'→p) = L(p', ω)
 * 
 * 如果p'与p互相可见并且 ω = p - p'，则p点的BSDF可以表示为
 * 
 *      f(p''→p'→p) = f(p', ωo, ωi)
 *
 * 其中 ωo = p'' - p' , ωi = p - p'
 *
 */
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
