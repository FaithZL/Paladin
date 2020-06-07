//
//  volpt.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/21.
//

/**
 * 光在参与介质中的传输
 * 
 * 参考章节
 * http://www.pbr-book.org/3ed-2018/Light_Transport_II_Volume_Rendering/Volumetric_Light_Transport.html
 *
 * 这些采样方式让在参与介质中的各种光照传输得到了实现的可能
 * 在integrator.hpp文件中的estimateDirectLighting函数可以处理在参与介质中的情况
 *
 * 在采样到一个光源的时候，如果采样得到的interaction是在参与介质中，那么有必要计算它对于
 * 某个给定出射方向与入射方向的相函数并计算对应对应方向的PDF，来执行符合重要性采样。
 *
 * VolumePathTracer (体渲染路径追踪) 包含了光线在参与介质中的衰减与散射计算，同样，也包含
 * 物体表面的散射计算，VolumePathTracer的大体结构与PathTracer非常相似，所以，我们在这里只
 * 讨论与VolumePathTracer与PathTracer相比有哪些不同。假设读者已经了解的PathTrancer的基本原理
 *
 * 在采样散射路径的每个步骤中，ray首先与场景中的表面求交，找到最近的表面相交点(如果有的话)
 * 接下来调用medium中sample方法去计算参与介质，如果路径中的下一个顶点在参与介质中，则
 * 初始化传入的MediumInteraction对象，任意一种情况，。。。。(说实话这个原文很难翻译，但我看得懂)
 * 
 *
 * 
 */

#ifndef volpath_hpp
#define volpath_hpp

#include "core/integrator.hpp"
#include "math/lightdistribute.hpp"

PALADIN_BEGIN

// 思路基本与路径追踪一致，只是添加了参与介质的渲染
class VolumePathTracer : public MonteCarloIntegrator {

public:
	VolumePathTracer(int maxDepth, std::shared_ptr<const Camera> camera,
                   std::shared_ptr<Sampler> sampler,
                   const AABB2i &pixelBounds, Float rrThreshold = 1,
	               const std::string &lightSampleStrategy = "power");

	virtual void preprocess(const Scene &scene, Sampler &sampler) override;

    virtual nloJson toJson() const override {
        return nloJson();
    }

    Spectrum _Li(const RayDifferential &ray, const Scene &scene,
                        Sampler &sampler, MemoryArena &arena, int depth) const;
    
    Spectrum rayIntersectAndLookForLight(const Scene &scene,
                                           Sampler &sampler, const RayDifferential &ray,
                                           int maxInteraction,  Interaction *intr,
                                           const Medium &medium) const;
    
    virtual Spectrum Li(const RayDifferential &ray, const Scene &scene,
                Sampler &sampler, MemoryArena &arena, int depth) const override;

private:
	// 最大反射次数
	const int _maxDepth;
	// 俄罗斯轮盘结束的阈值
    const Float _rrThreshold;
    // 光源采样策略
    const std::string _lightSampleStrategy;
    // 光源分布
    std::unique_ptr<LightDistribution> _lightDistribution;
    
    std::vector<int> _nLightSamples;
};

USING_STD;

CObject_ptr createVolumePathTracer(const nloJson &param, const Arguments &lst);

PALADIN_END

#endif /* volpt_hpp */
