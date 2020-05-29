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
 * Lo(p, ωo) = Le(p, ωo) + ∫[hemisphere]f(p, ωi, ωo)Lo(t(p, ωi), -ωi)|cosθi|dωi  1式
 * 
 * 可以看到，目前LTE是对方向的积分，因为顺着方向一定可以找到空间中的一个点
 * 我们可以把对方向的积分转化为对顶点的积分，方向积分之后就是立体角，点积分之后就是面积
 * (说实话我表达得很不好，哎，水平太低，但是看了表达式之后应该就懂了！！！)
 * 
 * 我们定义光线离开点p'到p的辐射度为 
 *
 * 		L(p'→p) = L(p', ω)           2式
 * 
 * 如果p'与p互相可见并且 ω = p - p'，则p点的BSDF可以表示为
 * 
 *      f(p''→p'→p) = f(p', ωo, ωi)
 *
 * 其中 ωo = p'' - p' , ωi = p - p'
 *
 * 	又由    dA = (r^2 dω) / |cosθ'| , r = |p-p'|       
 *
 * 可得
 *            |cosθ| |cosθ'|
 *  dω = dA  ----------------       其中θ'为方向ω与dA法线的夹角   3式
 *               |p-p'|^2
 *
 * 我们用函数G(p'↔p)取代dA的系数，G(p'↔p)形式如下
 * 
 *                    |cosθ| |cosθ'|
 *      G(p'↔p) =  --------------------
 *                      |p - p'|^2
 *
 * 带入3式到1式中，得：
 * 
 * Lo(p'→p) = Le(p'→p) + ∫[A] f(p''→p'→p) Lo(p''→p') G(p'↔p'') dA(p'')     4式
 * 
 * 以上方程就是LTE的路径形式
 *
 * 但上述方程有个问题，如果p'与p互相不可见，G(p'↔p)值为0
 * 
 * 所以我们完善一下G(p'↔p)函数
 *
 *                    |cosθ| |cosθ'|
 *      G(p'↔p) =  ------------------- V(p'↔p)   
 *                      |p - p'|^2
 *
 * V(p'↔p) 为可见测试函数，如果两点相互可见，值为1，否则值为0
 *
 *   ***** 需要注意的一点是4式积分空间为场景中所有点
 *
 * L(p1→p0) = Le(p1→p0) + ∫ L(p2→p1) f(p2→p1→p0) G(p1↔p2) dA(p2)
 * L(p2→p1) = Le(p2→p1) + ∫ L(p3→p2) f(p3→p2→p1) G(p2↔p3) dA(p3)
 * L(p3→p2) = Le(p3→p2) + ∫ L(p4→p3) f(p4→p3→p2) G(p3↔p4) dA(p4)
 * L(p4→p3) = Le(p4→p3) + ∫ L(p5→p4) f(p5→p4→p3) G(p4↔p5) dA(p5)
 * L(p5→p4) = Le(p5→p4) + ∫ L(p6→p5) f(p6→p5→p4) G(p5↔p6) dA(p6)
 * L(p6→p5) = Le(p6→p5) + ∫ L(p7→p6) f(p7→p6→p5) G(p6↔p7) dA(p7)
 *
 * 进一步迭代之后，可以写成如下形式
 *
 * L(p1→p0) = Le(p1→p0) 
 *   + ∫ Le(p2→p1) f(p2→p1→p0) G(p1↔p2) dA(p2)
 *   + ∫ ∫ Le(p3→p2) f(p3→p2→p1) G(p2↔p3) f(p2→p1→p0) G(p1↔p2) dA(p3) dA(p2)
 *   + ∫ ∫ ∫ Le(p4→p3) f(p4→p3→p2) G(p3↔p4) f(p3→p2→p1) G(p2↔p3) f(p2→p1→p0) G(p1↔p2) dA(p2) dA(p3) dA(p4)
 *   + ..............                    
 *                                       5式
 * 
 * 这个形式看起来太蛋疼了，简化一下，写成无穷级数的形式
 *                 
 *                 ∞   _
 * 	    L(p1→p0) = ∑ P(pn)                6式   
 *                n=1
 *     _
 * 其中pn = p0,p1,p2......pn，表示由n+1个点构成的长度为n的路径，其中p0在像平面上，pn在光源上
 *
 *   _                         n-1
 * P(pn) = ∫∫...∫ Le(pn→pn-1) ( ∏ f(pi+1 → pi → pi-1) G(pi+1 ↔ pi) ) dA(p2)...dA(pn)         7式
 *          n - 1              i=1
 *
 * 把中间bsdf与几何函数的乘积这部分抽出来，形式如下，我们把它称为路径的吞吐量 (throughout)
 * 它描述了光线从光源到相机所经过的所有顶点散射的辐射度因数
 *
 *    _     n-1
 *  T(pn) =  ∏ f(pi+1 → pi → pi-1) G(pi+1 ↔ pi) ) dA(p2)...dA(pn)        8式
 *          i=1
 * 
 * 显然，我们可以用蒙特卡洛方法对6式进行估计，但如果遇到一种情况，如果遇到狄拉克delta分布的光源，
 * 狄拉克函数是无法用常规随机方式采样的，如何处理？
 * 以点光源为例，由狄拉克函数定义可得
 *   _
 * P(p2) = ∫ Le(p2→p1) f(p2→p1→p0) G(p2↔p1) dA(p2)
 * 
 *          δ(plight - p2) Le(plight→p1)
 *       = ------------------------------ f(p2→p1→p0) G(p2↔p1)
 *                    p(p2)
 * 
 * 显然在蒙特卡洛估计中，p2的概率密度函数也是狄拉克函数，上下两个狄拉克函数互相抵消
 * 
 *      
 *       =  Le(plight→p1) f(p2→p1→p0) G(p2↔p1)
 * 
 * 不需要使用蒙特卡洛估计，直接求值即可
 * 
 * 
 *                 ∞   _
 * 	    L(p1→p0) = ∑ P(pn)                6式   
 *                n=1
 * 
 * 
 * 现在我们面临两个问题
 * 
 *  1. 6式是一个无穷级数，我们应该如何用有限计算量去估计一个无穷级数呢？
 *                      _
 *  2. 给定一个特定的项P(pn)，我们如何生成一个或多个路径来计算它的多维积分的蒙特卡罗估计。
 * 
 * 对于问题1，我们可以采用俄罗斯轮盘(Russian roulette)的方式去估计无穷级数
 * 原理如下，对于一个数 F，我们可以对F重新估计
 *
 *          F - qc
 *        ----------  (ξ > q)     
 *          1 - q 
 * F' =       
 *            c      (otherwise)
 *
 * 
 *                    E[F] - qc
 * E[F'] = (1 - q) ---------------- + qc = E[F] 
 *                      1 - q
 * 
 * 我们可以看到，重新估计之后，期望不变，但方差是会提高的(除非 c = F)
 * 所以我们必须慎重的选择q值
 * 
 * 将Russian roulette应用在6式中，可得
 * 
 *                1       _         1       _        1        _   
 * L(p1→p0) = -------- (P(p1) + -------- (P(p2) + -------- (P(p3) ...........
 *             1 - q1            1 - q2            1 - q3
 *            
 * 主要意思是，某个路径有可能被随机终止，但如果没有被终止的话，会有适当的加权，可以保证期望不变
 * 
 * 接下来要介绍的是如何采样路径
 * 
 * 我们需要用i+1个顶点表示一个长度为i的路径
 * 其中p0在像平面上，pi在光源上。
 *
 * 要求解场景中物体表面积的多重积分
 * 最自然的做法是根据场景中物体的表面积对顶点进行采样
 * 
 * 假如场景中有 i 个物体，每个物体的表面积为Ai，则采样到第 i 个物体的概率是 
 * 
 *   pi = Ai / (∑jAj)
 * 
 * 在第 i 个物体上采样到任意一点的PDF为 1/Ai
 * 
 * 则在场景中采样到任意一点的PDF为
 *
 * p = 1 / (∑jAj)   场景中的每个点是等概率的
 * 
 * 虽然我们可以使用同样的技术用于采样路径顶点到灯上的采样点，但这将导致高方差
 * 因为所有路径的pi如果没有落在光源上，则没有贡献。虽然期望值仍然是积分的正确值
 * 但收敛速度非常慢。有一个更好的办法是，仅仅对光源区域的方向采样，对应的PDF也要进行相应的调整
 * 给定一个完整的路径，则已经了所有我们需要估计的路径信息
 *
 * 如果我们知道一些物体的间接照明对场景中的大部分照明起了作用，
 * 我们可以分配一个更高的概率来生成这些对象上的路径顶点pi，并适当地更新样本权值
 * 然而，这样的采样路径有两个相互关联的问题。第一个可能导致高方差，而第二个可能导致不正确的结果。
 * 
 * 第一个问题是，许多路径如果有一对互不可见的邻接顶点，那么它们就没有贡献。
 * 考虑在一个复杂的建筑模型中应用这个区域抽样方法:路径中的相邻顶点之间几乎总是有一到两个墙，
 * 对路径没有贡献，估计中有很大的方差。
 *
 * 第二个问题是，如果被积函数中有狄拉克函数(例如，点光源或完全镜面BSDF)，
 * 这种采样技术将永远无法选择路径顶点，使得脉冲分布为非零。
 * 即使没有δ分布,随着BSDFs越来越光滑的几乎所有的路径将贡献较低,
 * 因为在f (pi+1 → pi → pi−1)将导致BSDF有很小的值或零值，将产生高方差。
 * 同样，如果不显式采样，小面积光源也可以是方差源。
 *
 * 解决这两个问题的一个解决方案是增量地构造路径，从摄像机p0的顶点开始。
 * 在每个顶点采样BSDF，生成一个新的方向;下一个顶点pi+1是通过在采样方向上追踪pi的射线
 * 并找到最近的交点来找到的。我们通过做出一系列选择，寻找具有重要的局部贡献的方向，
 * 从而有效地找到一条总体贡献较大的路径。虽然可以想象这种方法可能无效的情况，但它通常是一种很好的策略。
 *
 * 因为这种方法构造的路径通过抽样BSDF根据立体角,因为路径积分LTE是一个对面积的积分,
 * 我们需要将概率密度的从立体角pω转化为pA
 * 
 *             |cosθi|
 * pA = pω ---------------
 *          |pi - pi+1|^2
 *
 * 因为采样的两个顶点互相可见，G函数中的V函数始终为1， 
 * pA 与 G函数，除了 cosθi+1 项以外，其他项可以抵消，所以一条路径的蒙特卡洛估计如下
 *
 *     _
 * E(P(pN)) = Le(pi→pi-1) f(pi → pi-1 → pi-2) G(pi ↔ pi-1) / pA(pi)
 *
 *             i-2   f(pj+1 → pj → pj-1) |cosθj|
 *          *   ∏  --------------------------------            9式
 *             j=1          pω(pj+1 - pj)
 *             
 * 观察上式，可能会有一个疑问，为何第i项的G函数没有展开出来抵消pA?
 * 因为这一项比较特殊，需要估计直接光照，也就是估计整个场景的所有光源对于pi的直接光照贡献
 *
 * 路径的蒙特卡洛估计形式我们已经求出来了，可以开始写代码了
 *
 *                 ∞   _
 * 	    L(p1→p0) = ∑ P(pn)                6式   
 *                n=1
 *
 * 再来回顾一下6式，假设经过俄罗斯轮盘之后，我们需要计算 n 条路径的积分
 * 分别是长度为1，2，3...n，的n条路径的积分，我们把它称为a1,a2,a3.....an
 *
 * 比较直接的想法，从a1到an分别独立采样，但事实上我们不会这么做
 * 我们采样a3的时候，会复用a2的路径，采样a4的时候会复用a3的路径，归纳下来
 * 我们采样ai的时候会复用到ai-1的路径，简言之就是递增采样，这样采样会有什么不好？
 *
 * 书上说是降低了结果的质量，个人理解是关联采样，提高了方差。
 * 但由于采样速度快，收敛快，从而补足了这个缺陷
 * 
 * 接下来的采样思路，看代码吧
 */
class PathTracer : public MonteCarloIntegrator {
public:
	PathTracer(int maxDepth, std::shared_ptr<const Camera> camera,
                   std::shared_ptr<Sampler> sampler,
                   const AABB2i &pixelBounds, Float rrThreshold = 1,
               const std::string &lightSampleStrategy = "power");


	/**
	 * 预处理阶段，先构造好光源分布对象
	 * @param scene   场景对象
	 * @param sampler 采样器
	 */
	virtual void preprocess(const Scene &scene, Sampler &sampler) override;
    
    virtual nloJson toJson() const override {
        return nloJson();
    }
	
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

CObject_ptr createPathTracer(const nloJson &param, const Arguments &lst);

PALADIN_END

#endif /* pathtracer_hpp */
