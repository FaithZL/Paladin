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
 *      G(p'↔p) =  ----------------
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
 *      G(p'↔p) =  ---------------- V(p'↔p)   
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
