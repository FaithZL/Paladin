//
//  integrator.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef integrator_hpp
#define integrator_hpp

#include "header.h"
#include "scene.hpp"
#include "primitive.hpp"
#include "spectrum.hpp"
#include "light.hpp"
#include "bxdf.hpp"
#include "sampler.hpp"
#include "material.hpp"
#include "core/cobject.h"
#include "tools/classfactory.hpp"

PALADIN_BEGIN

/**
 * 先简单介绍一下积分器
 * 所有渲染的问题本质上是求解积分的问题
 * 场景文件中记录着场景中的光源数据，以及几何信息，每个表面的材质，相机的参数等等
 * 
 * 渲染也可以理解为计算几何表面上某个点在对应像素上的颜色贡献。
 * 
 * 渲染方程如下所示
 *    Lo(p, ωo) = Le(p, ωo) + ∫[hemisphere]f(p, ωi, ωo)Li(p, ωi)|cosθi|dωi   1式
 * 
 * Lo(p, ωo)为p点向ωo方向贡献的辐射度
 * 
 * 假设场景中没有参与介质，辐射度在传播过程中不会改变，则
 * 
 *    Li(p, ω) = Lo(t(p, ω), -ω)    2式
 *
 * 其中t(p, ω)为，在p点沿着ω方向上找到的第一个点p'
 *
 * 将1式带入2式得光照传输方程(LTE)
 *
 *   Lo(p, ωo) = Le(p, ωo) + ∫[hemisphere]f(p, ωi, ωo)Lo(t(p, ωi), -ωi)|cosθi|dωi  3式
 *
 * 我们可以看到，以上方程是一个积分方程，并且需要求解的Lo同时存在于等号的两边，
 * 这对我们的求解工作产生了很大的困难
 *
 * 绝大多数情况下，渲染方程无法求出解析解，只能求出数值解。
 *
 * 我们假定一个非常简单的情况
 * 假设场景在一个球体的内部,球面内部任意一点的出射辐射分布与其他任意一点相同，
 * 球体表面brdf为朗博反射分布，
 * 
 *         f(p, ωi, ωo) = c     4式
 *
 *  带入LTE之后，得
 *  
 *         Lo(p, ωo) = Le(p, ωo) + c ∫[hemisphere]Lo(t(p, ωi), -ωi)|cosθi|dωi
 * 
 * 化简 得     L = Le + cπL
 * 我们用ρhh替换cπ，
 *
 *    L = Le + ρhh(Le + ρhh(Le + ρhh(.....))) = ∑[i=0,∞]Le * ρhh^i
 *
 * 由于能量守恒ρhh小于1，级数收敛，
 *
 *    L = Le / (1 - ρhh)
 *
 * 以上为LTE最简单的一种情况，，有何作用？主要是为了调试我们写的积分器
 * 比对求出的数值解是否与解析解一致
 * 参考资料
 * http://www.pbr-book.org/3ed-2018/Light_Transport_I_Surface_Reflection/The_Light_Transport_Equation.html
 * 
 *
 * 现在来介绍一下LTE的求解方式
 * 经过不断的带入，我们会得到一个高维的积分，理论上还是无线维的，像这样高维的积分如何求解？
 * 显然黎曼积分的做法基本是不可能的，假如一个维度采5个样本，那么10个维度采样5^10个，引起维数爆炸
 *
 * 现在要来介绍一个神奇方法，蒙特卡洛方法
 * 先来推导一下蒙特卡洛积分的表达式
 *
 * 假如我们需要求解f(x)在[a,b]区间上的积分，
 * 先来看看，连续随机变量的期望公式为
 *
 *      E(X) = ∫[-,+]xp(x)dx  (其中p(x)为X的概率密度函数)  5式
 *
 * 为了求解 I = ∫[-,+]f(x)dx 这个表达式的积分值
 * 给定一组均匀随机变量 Xi ∈ [a,b]，则蒙特卡洛估计器的期望值为
 * 
 *          b - a  N
 *    FN = ------- ∑ f(Xi)               6式
 *            N   i=1
 *
 * 实际上，FN的期望等于积分值I，证明如下
 *
 *  E[FN] = E[(b-a)/N ∑[i=1,N]f(Xi)]
 *  
 *        = (b-a)/N ∑[i=1,N]E[f(Xi)]
 *        
 *        = (b-a)/N ∑[i=1,N] ∫[a,b]f(x)p(x)dx
 *        
 *        = 1/N ∑[i=1,N] ∫[a,b]f(x)dx
 *        
 *        = ∫[a,b]f(x)dx
 *        
 * 以上的p(x)函数可以由均匀分布放宽至任何分布，这个十分重要，因为谨慎的选择一个pdf可以减少方差
 * 我们来推导一下蒙特卡洛积分的方差
 *
 * 在推导之前，首先明确一点，各个服从独立同分布的的样本随机变量，
 * 它们的方差都相等啊，都等于它们服从的总体分布的方差啊，有了这一点，就可以推了!
 * 
 * 根据方差基本公式  V[X + Y] = V[X] + V[Y]  (X Y相互独立)
 *                V[aX] = a^2 * V[X]
 * 可得
 * 
 * V[IN] = V[1/N ∑[i=1,N] f(Xi)/p(Xi)]
 * 
 *       = 1/(N*N) V[∑[i=1,N] f(Xi)/p(Xi)]
 *       
 *       = 1/(N*N)∑[i=1,N] V[f(Xi)/p(Xi)]
 *
 *       = 1/N * V[f(X)/p(X)]
 *
 *       = 1/N * ∫((x - E[X])^2)f(x)dx
 *
 *       = 1/N * ∫((f(x)/p(x) - I)^2)f(x)dx         7式
 *
 * 可以看出如果 f(x)/p(x) 为一个常数时，这个常数必然等于I，此时方差为零，该估计为完美估计
 * 但完美估计是不存在的，我们只能尽可能的减少方差，就是使pdf函数与原函数的形状尽可能接近！
 *
 * 总结一下
 *     f(X)/p(X)的期望就是f(x)的积分值
 *     1/N * ∫((f(x)/p(x) - I)^2)f(x)dx就是蒙特卡洛估计的方差
 *
 * 这跟黎曼和求积分的方式比，好在哪？好处在于蒙特卡洛积分不需要因为维数的增长而采更多样本
 * 举个例子，我们需要求解三元函数的积分
 *
 *      I = ∫[x1,x2]∫[y1,y2]∫[z1,z2]f(x,y,z)dxdydz        8式
 *      
 * 我们可以在xyz三个变量对应的范围之内随机采样[x1,x2] [y1,y2] [z1,z2]
 * 假设三维随机变量Xi = (xi,yi,zi)
 * 如果每个维度都是均匀分布，则
 * 
 *                   1
 * p(X) = -----------------------               9式
 *         (x2-x1)(y2-y1)(z2-z1)
 *
 *       (x2-x1)(y2-y1)(z2-z1)
 * EN = ----------------------- ∑ f(Xi)           10式
 *                N
 *
 * 可以看出，样本的数量N可以任意选择，而不考虑被积函数的维数
 * 从而避免了维数爆炸
 * 参考资料
 * http://www.pbr-book.org/3ed-2018/Monte_Carlo_Integration/The_Monte_Carlo_Estimator.html
 *
 * 上文已经说了减少方差的方式就是使pdf与被估计的函数形状比较接近，对于bsdf的表达式
 * 我们通常可以求出解析解，从而可以找到一个接近bsdf形状的pdf(通常是使用对应的NDF，法线分布函数)
 * 但要估计光照方程
 *
 * Lo(p, ωo) = Le(p, ωo) + ∫[hemisphere]f(p, ωi, ωo)Lo(t(p, ωi), -ωi)|cosθi|dωi
 *
 * 方程中右半部分的积分简化来看可以替换成  ∫f(x)g(x)dx
 *
 * 我们是要估计f(x)g(x)函数的积分，我们只能找到与f(x)接近的pdf，也可以找到与g(x)接近的pdf
 * 但我们不知道f(x)g(x)函数接近的pdf，怎么办？？？？？？
 * 
 * 现在就要介绍一下复合重要性采样(MIS，Multiple Importance Sampling)
 * 假设f(x)的pdf为p1，g(x)的pdf为p2
 *
 * 则 ∫f(x)g(x)dx 的蒙特卡洛估计形式的表达式如下
 *
 *   1  nf   f(Xi)g(Xi)wf(Xi)     1  nf   f(Xi)g(Xi)wg(Xi)
 * ----  ∑ ------------------ + ----  ∑ ------------------     11式
 *  nf  i=1      p1(Xi)          nf  i=1      p2(Xi)  
 *
 * 其中wf，wg分别为f与g的权重函数，形式如下
 * 
 *           nf * pf(x)
 * wf(x) = --------------    (wg类似)    12式
 *           ∑ ni pi(x)
 *
 * 带入f(x)g(x)之后得到  
 *
 *                nf * pf(x)
 * wf(x) = ------------------------    13式  (wg同理)
 *          nf * pf(x) + ng * pg(x)
 *
 * 这个方法称为平衡式启发 (balance heuristic)
 * 将12式带入11式之后就可以得到一个低方差的估计样本了 
 * 
 * 实际上，还有一个更好的权重函数(power heuristic)
 *
 *
 *           (nf * pf(x))^β
 * wf(x) = ------------------       14式
 *           ∑ (ni pi(x))^β
 *           
 * paladin渲染器中提供了这两个权重函数，以便作对比
 * 
 * 参考资料
 * http://www.pbr-book.org/3ed-2018/Monte_Carlo_Integration/Importance_Sampling.html#MultipleImportanceSampling
 * 
 */
class Integrator : public CObject {
    
public:
    virtual ~Integrator() {
        
    }
    virtual void render(const Scene &) = 0;
    
    void outputSceneInfo(const Scene &scene, const Camera * camera = nullptr) const {
        AABB3f sceneBound = scene.worldBound();
        cout << "total scene bound box is:" << sceneBound << endl;
        int num = scene.lights.size();
        cout << "light num is " << num << endl;
    }
};

/**
 * 均匀采样所有光源
 * @param  it            场景中的点
 * @param  scene         场景对象
 * @param  arena         内存池
 * @param  sampler       采样器
 * @param  lightSamples  光源样本索引列表
 * @param  handleMedia   是否处理参与介质
 * @return               辐射度
 */
Spectrum uniformSampleAllLights(const Interaction &it, const Scene &scene,
                                MemoryArena &arena, Sampler &sampler,
                                const std::vector<int> &lightSamples,
                                bool handleMedia = false);

/**
 * 按照指定分布随机采样一个光源
 * @param  it           场景中的点
 * @param  scene        场景对象
 * @param  arena        内存池
 * @param  sampler      采样器
 * @param  handleMedia  是否处理参与介质
 * @param  lightDistrib 光源分布
 * @return              辐射度
 */
Spectrum sampleOneLight(const Interaction &it, const Scene &scene,
                               MemoryArena &arena, Sampler &sampler,
                               bool handleMedia = false,
                               const Distribution1D *lightDistrib = nullptr);

/**
 * 用复合重要性采样进行直接光照的估计
 * @param  it          场景中的点
 * @param  uShading    用于采样着色点的随机变量，用于生成wi方向，采样bsdf
 * @param  light       光源对象
 * @param  uLight      用于采样光源表面的二维随机变量
 * @param  scene       场景对象
 * @param  sampler     采样器
 * @param  arena       内存池
 * @param  handleMedia 是否处理参与介质
 * @param  specular    是否考虑高光反射
 * @return             返回直接光照辐射度
 */
Spectrum estimateDirectLighting(const Interaction &it, const Point2f &uShading,
                                const Light &light, const Point2f &uLight,
                                const Scene &scene, Sampler &sampler,
                                MemoryArena &arena, bool handleMedia = false,
                                bool specular = false);

class MonteCarloIntegrator : public Integrator {
    
public:
    MonteCarloIntegrator(std::shared_ptr<const Camera> camera,
                         std::shared_ptr<Sampler> sampler,
                         const AABB2i &pixelBound)
    : _camera(camera),
    _sampler(sampler),
    _pixelBounds(pixelBound) {
        
    }
    
    virtual void preprocess(const Scene &scene, Sampler &sampler) {
        
    }
    
    virtual void render(const Scene &scene) override;
    
    /**
     * 返回当前ray采样到的辐射度       
     */
    virtual Spectrum Li(const RayDifferential &ray, const Scene &scene,
                        Sampler &sampler, MemoryArena &arena,
                        int depth = 0) const = 0;
    
    // 高光反射
    Spectrum specularReflect(const RayDifferential &ray,
                             const SurfaceInteraction &isect,
                             const Scene &scene, Sampler &sampler,
                             MemoryArena &arena, int depth) const;
    
    // 高光透射
    Spectrum specularTransmit(const RayDifferential &ray,
                              const SurfaceInteraction &isect,
                              const Scene &scene, Sampler &sampler,
                              MemoryArena &arena, int depth) const;
    
    
    
protected:
    // 相机
    std::shared_ptr<const Camera> _camera;
    // 采样器
    std::shared_ptr<Sampler> _sampler;
    // 像素范围
    const AABB2i _pixelBounds;
};

PALADIN_END

#endif /* integrator_hpp */
