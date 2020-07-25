//
//  adaptive.hpp
//  Paladin
//
//  Created by Zero on 2020/7/19.
//

#ifndef adaptive_hpp
#define adaptive_hpp

#include "core/integrator.hpp"

PALADIN_BEGIN

//http://luthuli.cs.uiuc.edu/~daf/courses/rendering/Papers-2/RTHWJ.article.pdf
//https://www.cs.umd.edu/~zwicker/publications/MultidimensionalAdaptiveSampling-SIG08.pdf
//http://ima.udg.edu/~rigau/publications/rigau03b.pdf

/*
 自适应采样，为了解决一个屏幕内所有像素都用同样的spp数量渲染的问题，
 有些像素收敛得比较好，方差比较低，但有些像素方差高，为了使样本产生更多的价值，
 我们可以将样本用于收敛比较慢的像素上，方差低到一个既定的阈值时，停止采样，
 这就是自适应采样的核心思想
 
 Evaluation of confidence intervals
 评估置信区间
 
 置信区间是指由样本统计量所构造的总体参数的估计区间。
 在统计学中，一个概率样本的置信区间（Confidence interval）
 是对这个样本的某个总体参数的区间估计。
 置信区间展现的是这个参数的真实值有一定概率落在测量结果的周围的程度，
 其给出的是被测量参数的测量值的可信程度，即前面所要求的“一个概率”
 
 P{ L ∈ [L1 : L2] } = 1 - α
 L1 : L2 为最小最大值范围，可以理解为这是置信区间
 α为显著性水平，如果α=0.05，那么置信度则是0.95或95%
 卧槽，忽然发现这是我没有接触过的知识
      _
 L1 = L - t(n - 1)_sub(1 - α/2) * sqrt(s^2/n)
      _
 L2 = L + t(n - 1)_sub(1 - α/2) * sqrt(s^2/n)
 
 where t(n − 1)1−α/2 is the 1 − α/2 quantile of the t ñdistribution
 t(n − 1)1−α/2
 
 t(n − 1)1−α/2 是n-1个自由度的t分布的1 − α/2分位，s^2为方差
 
 求解置信区间的步骤
 第一步：求一个样本的均值
 第二步：计算出抽样误差。经过实践，通常认为调查：100个样本的抽样误差为±10%；500个样本的抽样误差为±5%；1200个样本时的抽样误差为±3%。
 第三步：用第一步求出的“样本均值”加、减第二步计算的“抽样误差”，得出置信区间的两个端点
*/

class AdaptiveIntegrator : public MonteCarloIntegrator {
    
public:
    
    AdaptiveIntegrator(std::shared_ptr<const Camera> camera,
                       std::shared_ptr<Sampler> sampler,
                       const AABB2i &pixelBound,
                       MonteCarloIntegrator * integrator,
                       Float maxErr)
    : MonteCarloIntegrator(camera, sampler, pixelBound) {
        _subIntegrator.reset(integrator);
    }
    
    virtual void preprocess(const Scene &scene, Sampler &sampler);
    
    virtual void render(const Scene &);
    
    virtual Spectrum Li(const RayDifferential &ray, const Scene &scene,
                        Sampler &sampler, MemoryArena &arena,
                        int depth = 0) const {
        return _subIntegrator->Li(ray, scene, sampler, arena, depth);
    }
    
    bool isContinue(Sampler * sampler) const;
    
private:
    unique_ptr<MonteCarloIntegrator> _subIntegrator;
    
    Float _maxError;
    Float _quantile;
    Float _pValue;
    Float _averageLuminance;
    Float _maxSpp;
};

CObject_ptr createAdaptiveIntegrator(const nloJson &param, const Arguments &lst);

PALADIN_END

#endif /* adaptive_hpp */
