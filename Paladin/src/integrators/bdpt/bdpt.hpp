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
 *
 * 路径空间测量方程(The Path-Space Measurement Equation)
 *
 * 
 * I(j) = ∫[A_film] ∫[sphere] We_j(p_film, w) Li(p_film, w) |cosθ| dw dA(p_film)
 *
 *  = ∫[A_film] ∫[A] We_j(p0->p1) Li(p1->p0) G(p1<->p0) dA(p1) dA(p0)  1式
 *
 * I(j)第j个像素的值(可以理解为单一分量的颜色值)，
 * 积分域A为场景所有表面，A_film为film表面，p0为film上的点，p1为场景中的点
 *
 * 其中的We_j(p0->p1)函数为响应函数，形式如下
 *
 *     We_j(p0->p1) = f_j(p0) δ(t(p0, w_camera(p1)) - p1) 
 *
 * 用人话来说就是，We_j(p0->p1) 等于 p0处的滤波函数值 * p0与相机方向确定的一个点p1的狄拉克函数
 *
 * 1式实际上是单向路径追踪的渲染方程的完整形式，对于pt来说，每个p0所对应的p1是固定的，所以积分可以简化
 * pathtrancer.hpp文件中的4式，
 * 
 * 1式可能看起来很复杂，但是它给了我们一个重要的启示，学习双向方法必须要理解的概念
 *
 *
 * 带入pathtracer.hpp中的6式，我们把它展开
 *
 *                 ∞   _
 * 	    L(p1→p0) = ∑ P(pn)        (详见pathtracer.hpp文件，表示所有路径的L相加)
 *                n=1
 *
 *
 * I(j) = ∫[A_film] ∫[A] We_j(p0->p1) Li(p1->p0) G(p1<->p0) dA(p1) dA(p0) 
 *
 *    ∞                               _
 *  = ∑ ∫[A_film] ∫[A] We_j(p0->p1) P(pi) G(p1<->p0) dA(p1) dA(p0)
 *   i=1
 *
 * 将pathtracer.hpp文件中8式，带入后，得
 *
 *    ∞                                           _
 *  = ∑  ∫ [A_film] ∫[A] ....... ∫[A] We_j(p0->p1) T(pi) G(p1<->p0) dA(p_i+1) .... dA(p0)
 *   i=1                   n重
 *
 *
 * 要理解所有的双向方法，我们要树立一个概念就是，
 * 辐射的发射跟响应是对称的，Le表示光源发射的辐射度，We表示相机的感应度，
 * 这两项都没有特别处理，从这一点我们可以推断，发射和测量的概念在数学上是可以互换的
 * 暂时理解为，光的传播是对称的，路径可以互反
 * 也就是说pi传到p0的路径，同样反方向p0也可以传向pi
 * 
 * 这种对称性的含义很重要:它表示我们可以用两种不同的方式来考虑渲染过程。
 * 
 *     1.光可以从光源发出，在场景中反弹，最后到达相机感应器，We代表响应函数
 *     2.我们可以把传感器想象成一个虚量，当它到达光源时，就会产生一个测量值
 *
 * 这个想法不仅仅是一个理论构想:它可以应用到实践中。
 *
 * 通过简单地交换摄像机和光源的作用，我们可以创建一种称为粒子跟踪的方法，
 * 它跟踪来自光源的光线，从而递归地估计到达表面的incident importance(这个词组不知道怎么翻译比较好)。
 * 这本身并不是一种特别有用的渲染技术，但它构成了其他方法(如双向路径跟踪和光子映射)的基本组成部分。
 *
 * 
 *
 * 
 */
 


/**
 * 
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
 * 以上介绍完了不对称性的源头，并且给出了解决方案。
 *
 * 
 *
 * 
 * 
 */

#include "core/integrator.hpp"
#include "../bidir_util/func.h"

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


CObject_ptr createBDPT(const nloJson &param, const Arguments &lst);

PALADIN_END

#endif /* bdpt_hpp */
