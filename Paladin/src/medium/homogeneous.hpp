//
//  homogeneous.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/12/21.
//

#ifndef homogeneous_hpp
#define homogeneous_hpp

#include "core/medium.hpp"
#include "core/spectrum.hpp"

PALADIN_BEGIN


/**
 * 先来介绍一下均匀介质
 *
 * 参考资料
 * http://www.pbr-book.org/3ed-2018/Light_Transport_II_Volume_Rendering/The_Equation_of_Transfer.html
 * http://www.pbr-book.org/3ed-2018/Light_Transport_II_Volume_Rendering/Sampling_Volume_Scattering.html
 *
 * Ls(p,w) = Le(p,w) + σs(p,w) ∫[sphere] p(p,w,wi) Li(p,wi) dwi   1式
 *
 * 通常，介质不会发光，所以上式可以简化为
 *
 * Ls(p,w) = σs(p,w) ∫[sphere] p(p,w,wi) Li(p,wi) dwi     2式
 * 
 * 光线在有参与介质的场景中的传输方程如下
 * 
 * Li(p,w) = Tr(p0->p) Lo(p0,-w) + ∫[0,t_max] Tr(p+tw->p) Ls(p+tw,-w) dt  3式
 * 
 * 其中，p为观察点(ray的起点)，w为观察方向(ray方向)，p0为ray与场景物体的第一个交点
 * 那么，3式所表达的意义就非常明显了，这还是很容易理解的
 * 
 * 2式跟3式联合起来，就可以得到一个完整的光照传输方程了，
 * 
 * 其实Li(p,w)就可以理解为该像素接收的辐射度了
 * 
 * 那接下来的事情就显得比较简单了
 * 
 * 先来看3式，
 * 3式中的Tr(p0->p) Lo(p0,-w)项，很容易就搞定了，由于是均匀介质那么Tr函数只
 * 与两点之间的距离有关，Lo函数也可以用之前的方式计算出来。
 * 
 * 3式中的∫[0,t_max] Tr(p+tw->p) Ls(p+tw,-w) dt项，求积分嘛，依然用蒙特卡洛估计，
 * 显然是随机采样的是积分变量t，也是可以估计得到的
 * 
 * 再来看2式，显然还是用蒙特卡洛方法去估计积分，在全空间内随机采样方向
 * 
 * 知道了以上方式，我们可以随机采样生成介质中的MediumInteraction对象了
 * 采样逻辑在sample函数实现
 * 
 * 采样生成MediumInteraction对象时可能有两种情况
 * 1.采样生成的t不在[0,t_max]区间内，显然样本点在物体表面上，因为t_max落在物体表面上
 * 		只需要估计Tr(p0->p) Lo(p0,-w)项，则不生成MediumInteraction对象
 * 
 * 2.如果生成的t在[0,t_max]区间内，表示样本点在介质中，
 * 		则只需要估计积分项，并生成MediumInteraction对象
 * 
 * 
 * 
 * 假设函数pt(t)表示位置p+tw处每单位距离内生成一个interaction的概率密度函数
 * 由于可能不会采样到一个medium interaction，所以在[0,t_max]区间上的积分可能不为1，
 * 我们定义了一个p_surf表示对表面项进行采样的相关概率
 *
 *           p_surf = 1 - ∫[0,t_max] pt(t) dt
 *
 * 所以 ∫[0,t_max] pt(t) dt 可以理解为采样到介质的概率
 *       显然，两个概率值相加为1，没毛病
 *
 * 接下来解释一下sample函数，这与之前我们遇到的BSDF中的sample_f函数不同，主要在于
 * 这个sample函数不会像sample_f一样返回pdf，通常不需要。
 * 
 * 要想判断样本点是在物体表面还是在介质中，首先要得到t值
 * 
 * t是如何计算出来的呢？？？？？？？
 * 通过均匀介质的传输系数可得
 *		
 *      f(t) = e^(−σt * t)    4式
 * 
 * 
 * 
 * 
 * 
 * 逆变换算法可得
 * 
 *           - ln(1 - ξ)
 *      t = -------------
 *               σt
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */
class HomogeneousMedium : public Medium {
    
public:
    HomogeneousMedium(const Spectrum &sigma_s, const Spectrum &sigma_a, Float g)
    :_sigma_s(sigma_s),
    _sigma_a(sigma_a),
    _sigma_t(sigma_a + sigma_s),
    _g(g) {

    }

	virtual Spectrum Tr(const Ray &ray, Sampler &sampler) const override;

	virtual Spectrum sample(const Ray &ray, Sampler &sampler, MemoryArena &arena,
						MediumInteraction *mi) const override;

private:
	// 散射系数
	const Spectrum _sigma_s;
	// 吸收系数
	const Spectrum _sigma_a;
	// 传播系数
	const Spectrum _sigma_t;
	// 各向异性系数
	const Float _g;
};

PALADIN_END

#endif /* homogeneous_hpp */
