//
//  bdpt.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/12/31.
//

#ifndef bdpt_hpp
#define bdpt_hpp

/**
 * 双向路径追踪(bidirectional path tracing)简称bdpt
 *
 * 双向路径追踪跟路径追踪(path tracing 简称pt)相比，有什么好处？
 * 首先，例如镜面反射比较多的场景，如果用pt算法渲染，噪点会比较严重，原因如下
 * 因为镜面反射对环境产生的光照效果接近直接光照的效果，但进行光照估计的时候并没有把
 * 镜面当做光源估计直接光照，所以pt追踪的路径能不能击中光源就看运气了，就相当于仅仅
 * 根据BSDF分布瞎JB采样，尤其是光源面积较小，那结果就是收敛得特别慢，跟那啥99行的smallpt差不多
 *
 * pt还有一个问题，就是整个场景的光照几乎来自于间接光照，比如说光源大部分被遮蔽起来了
 * 这种情况下，每次估计直接光照时几乎都是无效的，所以，pt对于这类场景，全靠BSDF瞎JB采样，
 * 噪点自然就很多，需要大量的采样数量才能收敛得比较好。
 * 总的来说就是，从相机出发的射线很难找到光源。
 *
 * 现在就来介绍一下bdpt
 * 从相机发射射线的同时，也从光源发射射线，一旦发现相机出发的子路径能与光源出发的子路径相连接
 * 则认为这是一条有效路径，样本估计。
 *
 * 从相机发出的射线是模拟光线的反向传输，有些BSDF是不对称的，则需要特殊处理
 * 例如，高光透射，
 *
 * 假设f(wi->wo) 的互反函数为 f_adjoint(wi->wo) 
 * 则满足
 *                                   ηi
 * f_adjoint(wi->wo) = f(wo->wi) = (----)^2 f(wi->wo)
 *                                   ηo
 *
 * 以上表达式直接给出结论，推导过程详见 SpecularTransmission 的注释
 *
 * 除了BSDF以外，还有一些内容的不对称性也会导致不正确的结果
 * 着色法线
 * 观察以下方程
 *
 * 如果不考虑着色法线：
 *
 *      Lo(wo) = ∫ Li(wi) f_Ng(wi->wo) |wi · Ng| dwi
 *
 * 但如果有着色法线的情况下：
 * 
 *      Lo(wo) = ∫ Li(wi) f_Ns(wi->wo) |wi · Ns| dwi
 *
 * 但我们需要如下形式：
 *
 *      Lo(wo) = ∫ Li(wi) f(wi->wo) |wi · Ng| dwi
 *
 * 根据上述表达式，可得 f与f_Ns的关系为
 *
 *                              wi · Ns
 * f(wi->wo) = f_Ns(wi->wo) |-------------|
 *                              wi · Ng
 *
 * 进一步，可得
 * 
 *                              wo · Ns
 * f(wo->wi) = f_Ns(wo->wi) |-------------| = f_adjoint(wi->wo)
 *                              wo · Ng
 *
 * 假设 f_Ns(wi->wo) = f_Ns(wo->wi)，则
 * 
 *  f_adjoint(wi->wo)     wo · Ns     wi · Ng
 * ------------------ = |---------| |---------|
 *     f(wi->wo)          wo · Ng     wi · Ns
 *
 * 给每个f函数添加一个校正因数(correct factor cf)
 *
 *        wo · Ns     wi · Ng
 * cf = |---------| |---------|  (importance transport)
 *        wo · Ng     wi · Ns
 *
 * cf = 1 (radiance transport)
 * 
 * 以上介绍完了不对称性的源头，并且给出了解决方案
 * 
 */

#include "core/integrator.hpp"
#include "pathvertex.h"

PALADIN_BEGIN

class BidirectionalPathTracer : public Integrator {
    
    BidirectionalPathTracer(std::shared_ptr<Sampler> sampler,
                   std::shared_ptr<const Camera> camera, int maxDepth,
                   bool visualizeStrategies, bool visualizeWeights,
                   const AABB2i &pixelBounds,
                   const std::string &lightSampleStrategy = "power")
    : _sampler(sampler),
    _camera(camera),
    _maxDepth(maxDepth),
    _visualizeStrategies(visualizeStrategies),
    _visualizeWeights(visualizeWeights),
    _pixelBounds(pixelBounds),
    _lightSampleStrategy(lightSampleStrategy) {
      
    }
    
    virtual nloJson toJson() const override {
        return nloJson();
    }
    
    void render(const Scene &scene) override;
    
private:
    std::shared_ptr<Sampler> _sampler;
    std::shared_ptr<const Camera> _camera;
    const int _maxDepth;
    const bool _visualizeStrategies;
    const bool _visualizeWeights;
    const AABB2i _pixelBounds;
    const std::string _lightSampleStrategy;
};

PALADIN_END

#endif /* bdpt_hpp */
