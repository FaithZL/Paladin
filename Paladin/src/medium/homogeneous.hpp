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
 * 我们定义了一个p_surf表示对表面项进行采样的离散概率(重点)
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
 * 似乎把f(t)当做CDF就好了，仔细一看，卧槽，CDF是严格单调不减函数
 * f(t)显然不满足条件，这时，灵活的变换一下，改为
 *
 *      f(t) = 1 - e^(−σt * t)  
 *
 * 好了，f(t)现在是单调递增函数了
 * 
 * 对f(t)求反函数
 * 
 * 逆变换算法可得，其中ξ为[0,1)的均匀随机变量
 * 
 *           - ln(1 - ξ)
 *      t = -------------
 *               σt
 * 
 * t的采样方式就这样求出来了
 *
 * 对f(t)函数求导，可得对应的PDF，
 * 
 *     PDF = σt e^(−σt * t)
 * 
 * 现在来介绍一下3式的蒙特卡洛估计形式
 * 
 * Li(p,w) = Tr(p0->p) Lo(p0,-w) + ∫[0,t_max] Tr(p+tw->p) Ls(p+tw,-w) dt
 * 估计这个函数值的方法还是使用蒙特卡洛估计
 * 过程如下：
 *     首先通过随机采样生成参数t，判断采样到的点是在介质中还是落在物体表面上
 *       如果落在物体表面上，则估计Tr(p0->p) Lo(p0,-w)中的Tr函数，我们暂且称为吞吐量β
 *       根据离散蒙特卡洛估计的定义，被估计是函数需要除以PMF，概率质量函数，也就是对应概率值
 *       估计式如下
 * 
 *                    Tr(p0->p)
 *       β_surf = ------------------  (为何不包含Lo(p0,-w)，稍后会介绍到)
 *                     p_surf
 *
 *      
 *     如果判断采样的点是落在介质中，很明显啦，根据连续蒙特卡洛估计的定义就可以得出了
 * 
 *               σs(p+tw) Tr(p+tw->p)
 *      β_med = ----------------------   (为何不包含Li(p,wi)，稍后会介绍到)
 *                     pt(t)
 * 
 *     可能会有点疑问σs(p+tw)从哪里来的？？？？
 *     σs(p+tw)是从2式中来的
 * 
 * 好了，现在来总结一下对3式使用蒙特卡洛方法估值的过程
 *    先生成t，确定样本点落在物体表面上或介质中
 *    如果落在物体表面上，则估计第一项的值，一次估值结束
 *    如果样本点落在介质中，则估计第二项，估计第二项的时候需要估计Ls(p,w)的值
 *    同样是用蒙特卡洛估计，对方向采样，生成Ls(p,w)的样本值，此时才算是完成一次估计
 * 
 * sample函数相当于只执行了采样t的过程，
 *    1.如果采样到物体表面，则采样对应的bsdf估计Lo
 *    2.如果采样到介质，则采样对应相函数估计Ls(p,w)
 * 
 * 上诉1,2两个过程均在VolumePathTracer中的Li函数中执行，
 * 这就解释了为何 β_surf中不包含Lo(p0,-w)， β_med中不包含Li(p,wi)的原因
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
