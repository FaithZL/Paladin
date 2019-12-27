//
//  medium.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/7/16.
//  Copyright © 2019 Zero. All rights reserved.
//

/**
 *
 * 先来介绍一下基础的光线在参与介质中的散射问题
 * 
 * 参考章节
 * http://www.pbr-book.org/3ed-2018/Volume_Scattering/Volume_Scattering_Processes.html
 * 在有参与介质的环境中，有三个主要过程影响辐射的分布
 *      1.吸收:由于光转换成另一种形式的能量(如热)而引起的亮度降低
 *      2.辐射:发光粒子对环境产生的辐射
 *      3.散射:由于与粒子碰撞而散射到其他方向
 *
 * 1.吸收
 * σa为单位距离内光被吸收的概率密度。σa为关于位置与方向的函数，单位为1/m
 * 以下方程描述了光线通过一个微分距离dt，被参与介质吸收的过程
 * 
 *      Lo(p, ω) − Li(p, −ω) = dLo(p, ω) = −σa(p, ω) Li(p, −ω) dt   1式
 *      用人话来说就是入射辐射度减去出射辐射度等于被吸收的辐射度
 *  
 * 如果需要求解光线传播一段距离l之后的剩余辐射度百分比r，则可以列出以下微分方程
 *
 *      dr = −σa(p + tω, ω) r dt       2式
 *      用人话来说：剩余量的变化量 = 吸收率 * 剩余量 * 位置变化量
 *      
 *      求解得：r = e^(−∫[0,l]σa(p+tω,ω)dt)
 *
 * 2.发射
 * 某些粒子吸收辐射能量之后，由化学，热能，原子能转化为光能
 * 我们用 Le(p, ω) 表示在某介质中，光线在指定点p，指定方向ω，在单位距离上的产生的辐射度(注意是单位距离！)
 *          
 *      dL = Le(p, ω) dt     3式
 *      用人话来说：微分距离dt上产生的辐射等于Le(p, ω)乘以dt
 * 
 * 进一步，得到：
 * 
 *      dLo(p, ω) = Le(p, ω) dt         4式
 * 以上表达式包含一个假设，发出的光Le不依赖与入射光Li（我觉得我翻译得跟泡狗屎一样，自己理解得也不好，todo）
 *
 *
 * 3.向外散射
 *  参与介质中第三种基本的光相互作用是散射。当射线穿过介质时，
 *  它可能与粒子碰撞并向不同的方向散射。这对光束携带的总辐射有两个影响。
 *  由于光束的某些方向发生了偏转，使得光束的微分区域的辐射强度降低。这种效果称为向外散射
 *  但是，其他射线的辐射可能会散射到当前射线的路径中
 * 
 *  同样是光线通过一个微分区域
 *
 *      dLo(p, ω) = −σs(p, ω) Li(p, −ω) dt    5式
 *      用人话来说：出射方向的辐射度的微分变化量等于 入射辐射度乘以外散射系数乘以微分距离
 * 
 * 由于吸收以及外散射，辐射度的减少量为 
 *
 *       σt(p, ω) = σa(p, ω) + σs(p, ω)  6式
 * 
 * 其中 反射率(albedo) 
 *
 *              ρ = σs/σt   7式 
 * 
 * 显然反射率的值在0到1之间
 * 它描述了散射事件中散射(相对于吸收)的概率。
 * 第二个是平均自由路径,1/σt,表示光子在碰撞粒子之前平均行走过的距离
 * 结合向外散射与吸收，总体衰减量表达式如下
 *      
 *      dLo(p,ω)/dt = −σt(p, ω) Li(p, −ω)    8式
 *      
 * 假设光线从p传播到p',衰减之后剩余量为(注意，是原方向的衰减)
 * 
 *      Tr(p→p') = e^(−∫[0,l](σa + σs)(p+tω,ω)dt)   9式
 *
 * 如果是均匀介质，则可以简化为
 *
 *      Tr(p → p′) = e^(−σt * l)           9.1式
 *      
 * 如果衰减系数满足方向对称（绝大多数都是这个情况），得
 *
 *      Tr(p→p') = Tr(p'→p)      10式 
 *
 * 还有一个非常有用的特性
 *
 *      Tr(p→p'') = Tr(p→p') Tr(p'→p'')  11式  (显然的啦，自行脑补)
 *
 * 9式中的负指数成为光学厚度，记为
 *
 *      τ(p→p') = ∫[0,l](σa + σs)(p+tω,ω)dt   11式
 *
 * 4.向内散射
 *  上面已经介绍过了向外散射，向内散射类似，就是来自于其他方向的光线撞击粒子之后
 *  内散射解释了由于光线从其他方向散射而引起的沿射线的辐射增加。
 *  微分体积外部的辐射沿射线方向散射并添加到入射辐射。
 *
 * 假设粒子之间的距离至少是它们半径的几倍，则可以忽略介质粒子之间的相互作用
 * 在以上假设中我们定义一个phase函数p(ω,ω')，描述了散射辐射的角度在一个点的分布
 * 类似于bsdf，可能这样说不一定准确，
 * phase函数有一个归一化限制条件
 *
 *      ∫[sphere]p(ω,ω')dω = 1    12式
 *
 * 这个约束意味着相位函数实际上定义了在特定方向上散射的概率分布。
 *
 *      todo
 *      dLo(p, ω) = Ls(p, ω) dt  13式    (参照4式)
 *
 *
 * Ls(p, ω) = Le(p, ω) + σs(p, ω) ∫[sphere]p(p, ωi, ω) Li(p, ωi) dωi  14式
 *
 * 
 *
 *
 */

#ifndef medium_hpp
#define medium_hpp

#include "core/header.h"
#include "core/cobject.h"

PALADIN_BEGIN
/**
 *
 * 正如描述表面散射的BSDF模型种类繁多，许多相函数也得到了发展。
 * 这些模型从参数化模型(可用于将具有少量参数的函数拟合到测量数据)
 * 到基于从具有已知形状和材料的粒子(如球形水滴)推导出散射辐射分布的分析模型。
 * 相函数是一个一维函数，参数为cosθ，其中θ为入射方向与出射方向的夹角
 * 相函数有互换性，任意交换两个向量，函数值保持不变
 * 相函数满足以下表达式
 *       ∫[sphere]p(ωo,ωi) = 1
 * 所以各向同性的相函数满足
 * p(ωo,ωi) = 1/4π
 *
 * Henyey和Greenstein(1941)开发了一种广泛使用的相位函数。
 * 这种相位函数是专门设计的，以方便适合测量散射数据。
 * 一个单独的参数g(称为不对称参数)控制着散射光的分布
 *
 * 表达式如下
 *
 *                1              1 - g^2
 * P_hg(cosθ) = ---- * ----------------------------
 *               4π     (1 + g^2 + 2g cosθ)^(3/2)
 * 
 * 其中θ为入射方向与出射方向的夹角
 */
class PhaseFunction {
public:
   
    virtual ~PhaseFunction() {

    }

    virtual Float p(const Vector3f &wo, const Vector3f &wi) const = 0;

    virtual Float sample_p(const Vector3f &wo, Vector3f *wi,
                           const Point2f &u) const = 0;

    virtual std::string toString() const = 0;
};

// 介质
class Medium {
public:
    // Medium Interface
    virtual ~Medium() {}
    
    virtual Spectrum Tr(const Ray &ray, Sampler &sampler) const = 0;
    
    virtual Spectrum sample(const Ray &ray, Sampler &sampler,
                            MemoryArena &arena,
                            MediumInteraction *mi) const = 0;
};

/**
 *                1              1 - g^2
 * P_hg(cosθ) = ---- * ----------------------------
 *               4π     (1 + g^2 + 2g cosθ)^(3/2)
 *
 * 这里主要介绍一下相函数的采样思路
 *
 * 不同于BXDF，相函数不会返回PDF值，我们假设相函数采样的PDF完美符合该函数的分布
 * 并且相函数是归一化的 ∫[sphere]p(ω,ω')dω = 1 ，所以函数值与PDF保持一致
 *
 * 现在来介绍一下sample_p函数的实现思路
 *
 * 采样的方向可以分为两个维度，θ与φ，θ∈[0,π), φ∈[0,2π)
 *
 * 其中θ为入射方向与出射方向的夹角，可以把wo方向作为θ为零的方向，
 * 根据相函数的分布采样入射方向
 * 
 * 显然 p(φ) = 1/2π
 *
 *         1                    1 - g^2
 * cosθ = ---- [1 + g^2 - (-----------------)^2]  (todo待推导)
 *         2π                1 - g + 2 ξ g
 *
 * 
 * 
 */
class HenyeyGreenstein : public PhaseFunction {
public:
    HenyeyGreenstein(Float g) 
    : _g(g) {

    }

    virtual Float p(const Vector3f &wo, const Vector3f &wi) const override;

    virtual Float sample_p(const Vector3f &wo, Vector3f *wi,
                   const Point2f &sample) const override;

    virtual std::string toString() const override {
        return StringPrintf("[ HenyeyGreenstein g: %f ]", _g);
    }

private:
    // 各向异性系数
    const Float _g;
};

/**
 * Henyey和Greenstein开发的phase函数，表达式如下
 * 
 *                1              1 - g^2
 * P_hg(cosθ) = ---- * ----------------------------
 *               4π     (1 + g^2 + 2g cosθ)^(3/2)
 *  
 */
inline Float phaseHG(Float cosTheta, Float g) {
    Float denom = 1 + g * g + 2 * g * cosTheta;
    return Inv4Pi * (1 - g * g) / (denom * std::sqrt(denom));
}

// 两个介质的相交处，nullptr表示真空
struct MediumInterface {
    MediumInterface() : inside(nullptr), outside(nullptr) {}

    MediumInterface(const Medium *medium) : inside(medium), outside(medium) {

    }

    MediumInterface(const Medium *inside, const Medium *outside)
    :inside(inside),
    outside(outside) {

    }
    
    bool isMediumTransition() const {
        return inside != outside;
    }
    // 内部的介质
    const Medium *inside;
    
    // 外部的介质
    const Medium *outside;
};

PALADIN_END

#endif /* medium_hpp */
