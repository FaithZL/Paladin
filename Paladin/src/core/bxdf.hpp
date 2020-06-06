//
//  bxdf.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef bxdf_hpp
#define bxdf_hpp

#include "header.h"
#include "spectrum.hpp"
#include "material.hpp"
#include "math/sampling.hpp"
#include "math/frame.hpp"

PALADIN_BEGIN

/**
 * 参考章节 
 * http://www.pbr-book.org/3ed-2018/Reflection_Models/Specular_Reflection_and_Transmission.html
 * 
 * 依次有斯涅尔定律，菲涅尔定律，镜面反射，镜面折射的内容讲解
 * 
 * 描述如何反射或折射的模块
 * -----------------------------------------------------------
 * 	斯涅尔定律
 * 折射率的定义如下:
 * 介质的折射率描述了光在 真空中速度 与 该介质中速度 之比，由希腊字母η表示，读作eta
 * 斯涅尔定律可得 ηi * sinθi = ηt * sinθt，θ表示对应的入射角(出射角)
 * 斯涅尔定律是由费马原理推导出来的，费马原理简述如下
 * 又称“最短时间原理”：光线传播的路径是需时最少的路径。
 * 结合三角函数即可推导出斯涅尔定理
 * 真实世界中，折射率是跟波长有关的，但在处理渲染问题上，我们假设与波长无关
 * 因为这个因素对渲染结果影响不大，并且还可以简化数学模型，减少计算量
 *
 * --------------------------------------------------------------
 * 
 * 简述一下菲涅尔定律
 * 比如说一张纸的表面，一看就知道是漫反射
 * 但如果观察的角度很接近零度，那就有比较明显的反射效果了，可以实践一下
 * 总而言之，高光反射率是跟入射光的角度有关的
 *
 * 菲涅尔方程指定了入射光的两种偏振状态下材料的相应反射率。
 * 由于偏振的视觉效果在大多数环境中不明显，所以在实践中我们通常假设光是非偏振的
 * 我们简化了偏振模型，以1/2(r∥^2 + r⊥^2)来计算菲涅尔反射率
 * 其中r∥平行偏振光的反射率，r⊥为垂直偏振光的反射率
 *
 * 菲涅尔效应需要分3类材料讨论
 * 1.绝缘体，例如，水，空气，矿物油，玻璃等材质，通常这些材质折射率为1到3之间
 * 2.导体，电子可以在它们的原子晶格内自由移动，允许电流从一侧流向另一侧。
 *     当导体受到电磁辐射(如可见光)时，这种基本的原子特性会转化为一种截然不同的行为:
 *     导体会反射绝大部分的光，只有极小一部分会被物体吸收，并且仅仅在距离表面0.1微米处被吸收
 *     因此，只有非常非常薄的金属片，光才能穿透，在paladin工程中，我们忽略这种现象，只计算反射分量
 *     与绝缘体不同的是，导体的折射率由复数表示 η' = η + i*k(k表示吸收系数)
 * 3.半导体，例如硅，锗，在paladin工程中，我们不做处理
 * 
 * 以上两种材质都可以用同一组菲涅尔方程表示，但由于绝缘体的折射率是实数，比较简单
 * 所以我们更加倾向于对绝缘体的菲涅尔方程做特殊处理，以便提高运行效率
 * 
 * 绝缘体的菲涅尔效应表达式如下
 * 
 * 			ηt * cosθi - ηi * cosθt
 * r∥ = -------------------------------
 * 			ηt * cosθi + ηi * cosθt
 *
 * 			ηi * cosθi - ηt * cosθt
 * r⊥ = ------------------------------
 * 			ηi * cosθi + ηt * cosθt
 * 
 * Fr = 1/2(r∥^2 + r⊥^2)
 *
 * 导体的菲涅尔效应表达式如下
 * η' = η + i*k(有时候光会被材质吸收转化为热量，k表示吸收系数)
 * 
 *          a^2 + b^2 - 2a * cosθ + (cosθ)^2
 * r⊥ = ----------------------------------------
 *          a^2 + b^2 + 2a * cosθ + (cosθ)^2
 * 
 *              (a^2 + b^2)(cosθ)^2 - 2a * cosθ(sinθ)^2 + (sinθ)^4
 * r∥ = r⊥ * ----------------------------------------------------------
 *              (a^2 + b^2)(cosθ)^2 + 2a * cosθ(sinθ)^2 + (sinθ)^4
 *
 * 其中 a^2 + b^2 = √[(η^2 - k^2 - (sinθ)^2)^2 + (2ηk)^2]
 * 
 *             ηt + i * kt
 * η + i*k = ---------------    3式
 *             ηi + i * ki
 *             
 * 由于入射介质为绝缘体，吸收系数k为零，则3式简化后
 * 
 *             ηt + i * kt
 * η + i*k = --------------- 
 *                 ηi
 * 
 * Fr = 1/2(r∥^2 + r⊥^2)
 *             
 * 由于能量守恒，光线传输的能量为反射之前的 1 - Fr
 * 以上公式是著名物理学家菲涅尔由电磁场边值关系出发的理论推演得出的
 * 这表达式太特么复杂了
 * 至于推导过程我也就不凑热闹了......
 * 
 * -----------------------------------------
 * 镜面反射
 * 由BRDF定义可得
 * Lo(wo) = ∫f(wo, wi)Li(wi)|cosθi|dwi
 * 又因为我们可以用菲涅尔函数计算反射部分
 * Lo(wo) = ∫f(wo, wi)Li(wi)|cosθi|dwi = Fr(wr)Li(wr)
 * wr = R(wo,n)  R为反射函数，n为法线，
 * 注意！！！！！工程中所有wr都是是入射光方向的反方向，也就是从入射点指向光源位置的方向
 * θo = θr，所以Fr(wo) = Fr(wr)
 * 这类的BRDF可以用狄拉克函数来构造，同样，我们会利用狄拉克函数的如下特性
 * 
 * 			∫f(x)δ(x - x0)dx = f(x0)      1式
 *             
 * 直观的感觉，我们可以得出如下表达式 (只有wi等于wr时，BRDF函数值才不为零)
 * 
 *         f(wo, wi) = δ(wi - wr)Fr(wi)
 *         
 * 虽然这看起来是正确的，但带入渲染方程之后得到
 *
 *          Lo(wo) = ∫δ(wi - wr)Fr(wi)Li(wi)|cosθi|dwi
 *
 * 带入联合1式可得
 *
 *          Lo(wo) = Fr(wr)Li(wr)|cosθr|
 *
 * 观察上式，卧槽？？？居然多了一个|cosθr|，这是不对的
 * 所以，综合来如下表达式才是正确的
 * 
 *          Lo(wo) = Fr(wr)Li(wr)
 *         
 * 我们推导出菲涅尔函数与BRDF的关系如下
 * 
 *                 δ(wi - wr)Fr(wr)
 * f(wo, wi) = -----------------------
 *                     |cosθr|
 *   
 * -----------------------------------------------------------------------------
 * 接下来开始讲解Microfacet Models(微面元模型)
 * 参考资料http://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models.html
 * 许多基于几何光学的方法来建模表面反射和透射是将粗糙表面建模为一系列朝向各异的微观小平面的集合
 * 表面的朝向分布用统计学方式去描述
 * 基于microfacet的BRDF模型的工作原理是对来自大量的microfacet的光透射进行统计建模
 * 虽然镜面投射对于模拟半透明材料很有用， Oren–Nayar模型把微面元当成漫反射表面。
 * 但BRDF最常用的方式还是把微面元的反射当成是理想镜面反射处理
 * 
 * Oren–Nayar模型
 * 简单说明在在Oren–Nayar类的注释中
 * 
 */


/**
 * 绝缘体菲涅尔函数
 *
 * 			ηt * cosθi - ηi * cosθt
 * r∥ = -------------------------------
 * 			ηt * cosθi + ηi * cosθt
 *
 * 			ηi * cosθi - ηt * cosθt
 * r⊥ = ------------------------------
 * 			ηi * cosθi + ηt * cosθt
 * 
 * r∥平行偏振光的反射率，r⊥为垂直偏振光的反射率
 * Fr = 1/2(r∥^2 + r⊥^2)
 * 如果光线从较高折射率的介质中射向较低介质，并且入射角接近90°
 * 则无法传播到折射率较低的介质(我们称为入射角大于临界角)
 *
 * @param  cosThetaI 入射角的余弦值
 * @param  etaI      入射介质的折射率
 * @param  etaT      传播介质的折射率
 * @return           绝缘体的菲涅尔函数值
 */
Float FrDielectric(Float cosThetaI, Float etaI, Float etaT);

/**
 * η' = η + i*k(有时候光会被材质吸收转化为热量，k表示吸收系数)
 * 
 *          a^2 + b^2 - 2a * cosθ + (cosθ)^2
 * r⊥ = ----------------------------------------
 *          a^2 + b^2 + 2a * cosθ + (cosθ)^2
 * 
 *              (a^2 + b^2)(cosθ)^2 - 2a * cosθ(sinθ)^2 + (sinθ)^4
 * r∥ = r⊥ * ----------------------------------------------------------
 *              (a^2 + b^2)(cosθ)^2 + 2a * cosθ(sinθ)^2 + (sinθ)^4
 *
 * 其中 a^2 + b^2 = √[(η^2 - k^2 - (sinθ)^2)^2 + (2ηk)^2]
 * 
 *             ηt + i * kt
 * η + i*k = ---------------    3式
 *             ηi + i * ki
 *             
 * 由于入射介质为绝缘体，吸收系数k为零，则3式简化后
 * 
 *             ηt + i * kt
 * η + i*k = --------------- 
 *                 ηi
 * 
 * Fr = 1/2(r∥^2 + r⊥^2)
 * 
 * @param  cosThetaI 入射角余弦值
 * @param  etaI      入射介质折射率
 * @param  etaT      物体折射率
 * @param  k         吸收系数
 * @return           导体的菲涅尔函数值
 */
Spectrum FrConductor(Float cosThetaI, const Spectrum &etaI,
                     const Spectrum &etaT, const Spectrum &kt);

/**
 * 返回的是入射光线的反方向
 * 简单说一下推导过程，注释画图不方便   
 * 这里隐含的假设是n与ωo都是单位向量
 *
 * 在草稿纸上画图，ωi + ωo = OC，OC为ωi与ωo角平分线，与法线n方向一致
 * θ为OC与ωo夹角
 * OC/2 = |ωo| * cosθ * n/|n|
 * cosθ = dot(ωo,n)/|n||ωo|
 *
 * ωi = OC - ωo = 2(dot(ωo,n) * n/|n|) - ωo
 * 
 * 又因为n为单位向量
 *
 * 则ωi = 2(ωo · n)n - ωo
 * 
 * @param  ωo 出射光线方向，单位向量
 * @param  n  法线方向，单位向量
 * @return    注意这里返回的是入射光线的反方向ωi
 */
inline Vector3f reflect(const Vector3f &wo, const Vector3f &n) {
    return 2 * dot(wo, n) * n - wo;
}

/**
 * 折射函数，根据入射方向(入射光方向的反方向)wi计算折射方向
 * 推导过程就不写了，比较麻烦，
 * 大概思路就是画一个单位圆，入射光线与出射光线还有法线都是单位向量，圆心就是光线折射的位置
 * .....应该不难推
 * @param  wi  入射方向(入射光方向的反方向)
 * @param  n   法线方向
 * @param  eta 入射介质与传播介质的折射率之比
 * @param  wt  出射方向
 * @return     是否有光线射出
 */
inline bool refract(const Vector3f &wi, const Normal3f &n, Float eta,
                    Vector3f *wt) {

    Float cosThetaI = dot(n, wi);
    Float sin2ThetaI = std::max(Float(0), Float(1 - cosThetaI * cosThetaI));
    Float sin2ThetaT = eta * eta * sin2ThetaI;
    
    if (sin2ThetaT >= 1) {
        return false;
    }
    Float cosThetaT = std::sqrt(1 - sin2ThetaT);
    *wt = eta * -wi + (eta * cosThetaI - cosThetaT) * Vector3f(n);
    return true;
}

inline bool sameHemisphere(const Vector3f &w, const Vector3f &wp) {
    return w.z * wp.z > 0;
}

inline bool sameHemisphere(const Vector3f &w, const Normal3f &wp) {
    return w.z * wp.z > 0;
}

inline Spectrum schlickFresnel(const Spectrum &R, Float cosTheta) {
    auto pow5 = [](Float a) { return (a * a) * (a * a) * a; };
    return R + (Spectrum(1) - R) * pow5(1 - cosTheta);
}



/**
 * BRDF(Bidirectional Reflectance Distribution Function)
 * 双向反射分布函数，定义给定入射方向上的辐射照度（irradiance）如何影响给定出射方向上的辐射率（radiance）
 * 定义如下
 *                                          dLr(wr)
 * f(wi,wr) = dLr(wr) / dEi(wi)  = --------------------------
 *                                   Li(wi) * cosθi * dwi
 *
 * 表示反射方向上的radiance与入射方向上的irradiance的变化率的比例
 *
 * BTDF函数(Bidirectional transmission Distribution Function)
 * 其实BTDF的定义与BRDF差不多，只是BRDF用于描述反射，BTDF用于描述折射，就不再赘述了
 *
 * 以上两个函数都有共同的属性，共同的函数，所以定义了一个BxDF基类
 */
class BxDF {
    
public:
    virtual ~BxDF() {
        
    }
    
    BxDF(BxDFType type) : type(type) {
        
    }
    
    bool matchesFlags(BxDFType t) const {
        return (type & t) == type;
    }
    
    /**
     * 返回该对方向的BRDF值
     * 这个接口隐含了一个假设，假设光源的波长是可以分解的
     * 一个波长的能量经过反射之后不会转移到其他波长上
     * 这个函数可以处理大多数情况，但个别情况无法处理
     * 比如说理想高光反射，为狄拉克函数，需要特殊处理
     * @param  wo 出射方向
     * @param  wi 入射方向
     * @return    该BRDF的函数值
     */
    virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const = 0;
    
    virtual Spectrum f(const Vector3f &wo, const Vector3f &wi,
                       const SurfaceInteraction &si) const {
        
    }
    
    /**
     * 这个函数可以理解为:
     * 该BXDF函数的蒙特卡洛估计的一个样本值
     * 指定出射方向，根据随机样本点采样入射方向
     * 返回BRDF值，同时通过指针返回入射方向以及该方向的概率密度函数
     * @param  wo          出射方向
     * @param  wi          需要返回的入射方向
     * @param  sample      用于计算出射方向的样本点
     * @param  pdf         返回的对应方向上的概率密度函数
     * @param  sampledType BxDF的类型
     * @return             radiance
     */
    virtual Spectrum sample_f(const Vector3f &wo, Vector3f *wi,
                              const Point2f &sample, Float *pdf,
                              BxDFType *sampledType = nullptr) const;
    
    virtual Spectrum sample_f(const Vector3f &wo, Vector3f *wi,
                                const SurfaceInteraction &si,
                                const Point2f &sample, Float *pdf,
                              BxDFType *sampledType = nullptr) const {
        Spectrum ret = f(wo, *wi, si);
        if (ret.IsBlack()) {
            *wi = Vector3f(0,0,0);
            *pdf = 0;
        } else {
            *wi = cosineSampleHemisphere(sample);
            if (wo.z < 0) {
                wi->z *= -1;
            }
            *pdf = pdfDir(wo, *wi);
        }
        return ret;
    }
    
    /**
     * ρhd(wo) = ∫[hemisphere]f(p,wi,wo)|cosθi|dwi
     * BRDF函数在半球上的cos积分值
     * 用有限样本点去估算积分值
     * @param  wo       出射方向
     * @param  nSamples 样本个数
     * @param  samples  样本列表
     * @return          ρhd(wo)
     */
    virtual Spectrum rho_hd(const Vector3f &wo, int nSamples,
                            const Point2f *samples) const;
    
    /**
     * ρhh(wo) = (1/π)∫[hemisphere]∫[hemisphere]f(p,wi,wo)|cosθo * cosθi|dwidwo
     * 观察一下可以看出其实ρhh(wo)就是ρhd(wo)在半球积分上的值除以π(半球的cos权重的立体角为π)
     * 因此 ρhh(wo)可以理解为半球空间的平均反射率
     * @param  nSamples  样本点个数
     * @param  samplesWo wo的样本数组
     * @param  samplesWi wi的样本数组
     * @return           ρhh(wo)
     */
    virtual Spectrum rho_hh(int nSamples, const Point2f *samplesWo,
                            const Point2f *samplesWi) const;
    
    /**
     * 返回入射方向为wi，出射方向为wo对应的概率密度函数值(立体角空间)
     * @param  wi [入射方向]
     * @param  wo [出射方向]
     * @return
     */
    virtual Float pdfDir(const Vector3f &wo, const Vector3f &wi) const;
    
    virtual Spectrum eval(const BSDFSamplingRecord &rcd) const {
        NotImplementedError("eval");
    }
    
    virtual Spectrum sample(BSDFSamplingRecord * rcd, Float *pdf) const {
        NotImplementedError("sample");
    }
    
    virtual std::string toString() const = 0;
    
    const BxDFType type;
};

inline std::ostream &operator<<(std::ostream &os, const BxDF &f) {
    os << f.toString();
    return os;
}

/**
 * 如果想针对一个特定的BxDF执行缩放
 * ScaledBxDF是个很好的选择，对外接口就这些，跟BxDF差不多
 * 就不再赘述了
 * 在MixMaterial材质中比较常用
 */
class ScaledBxDF : public BxDF {
    
public:

    ScaledBxDF(BxDF *bxdf, const Spectrum &scale)
    : BxDF(BxDFType(bxdf->type)),
    _bxdf(bxdf),
    _scale(scale) {
        
    }
    
    virtual Spectrum rho_hd(const Vector3f &w, int nSamples, const Point2f *samples) const {
        return _scale * _bxdf->rho_hd(w, nSamples, samples);
    }
    
    virtual Spectrum rho_hh(int nSamples, const Point2f *samples1,
                 const Point2f *samples2) const {
        return _scale * _bxdf->rho_hh(nSamples, samples1, samples2);
    }
    
    virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const {
        return _scale * _bxdf->f(wo, wi);
    }
    
    virtual Spectrum sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample,
                              Float *pdf, BxDFType *sampledType) const {
        Spectrum f = _bxdf->sample_f(wo, wi, sample, pdf, sampledType);
        return _scale * f;
    }
    
    virtual std::string toString() const {
        return std::string("[ ScaledBxDF bxdf: ") + _bxdf->toString() +
        std::string(" scale: ") + _scale.ToString() + std::string(" ]");
    }
    
private:
    BxDF *_bxdf;
    Spectrum _scale;
};

/**
 * 菲涅尔基类
 */
class Fresnel {
public:

    virtual ~Fresnel() {
        
    }

    /**
     * @param  cosThetaI 入射角余弦值
     * @return    		 返回对应入射角的菲涅尔函数值
     */    
    virtual Spectrum evaluate(Float cosI) const = 0;
    
    virtual std::string toString() const = 0;
};

inline std::ostream &operator<<(std::ostream &os, const Fresnel &f) {
    os << f.toString();
    return os;
}

/**
 * 导体菲涅尔类
 * evaluate函数将返回对应入射角的菲涅尔函数值
 */
class FresnelConductor : public Fresnel {
    
public:
    /**
     * @param  cosThetaI 入射角余弦值
     * @return    		 返回对应入射角的菲涅尔函数值
     */
    virtual Spectrum evaluate(Float cosThetaI) const {
        return FrConductor(std::abs(cosThetaI), _etaI, _etaT, _kt);
    }
    
    FresnelConductor(const Spectrum &etaI, const Spectrum &etaT,
                     const Spectrum &kt)
    : _etaI(etaI), _etaT(etaT), _kt(kt) {
        
    }
    
    virtual std::string toString() const {
        return std::string("[ FresnelConductor etaI: ") + _etaI.ToString() +
        std::string(" etaT: ") + _etaT.ToString() + std::string(" k: ") +
        _kt.ToString() + std::string(" ]");
    }
    
private:
    Spectrum _etaI, _etaT, _kt;
};

/**
 * 绝缘体菲涅尔类
 * evaluate函数将返回对应入射角的菲涅尔函数值
 */
class FresnelDielectric : public Fresnel {
    
public:
    /**
     * @param  cosThetaI 入射角余弦值
     * @return    		 返回对应入射角的菲涅尔函数值
     */
    virtual Spectrum evaluate(Float cosThetaI) const {
        return FrDielectric(cosThetaI, _etaI, _etaT);
    }
    
    FresnelDielectric(Float etaI, Float etaT) : _etaI(etaI), _etaT(etaT) {
        
    }
    
    virtual std::string toString() const {
        return StringPrintf("[ FrenselDielectric etaI: %f etaT: %f ]", _etaI, _etaT);
    }
    
private:
    Float _etaI, _etaT;
};

/**
 * 100%反射所有入射光
 * 尽管这是在物理上不可能的，但确实可以明显减少计算量
 */
class FresnelNoOp : public Fresnel {
public:
    virtual Spectrum evaluate(Float) const { 
    	return Spectrum(1.); 
    }
    
    virtual std::string toString() const {
        return "[ FresnelNoOp ]";
    }
};

class FresnelSchlick : public Fresnel {

public:
    FresnelSchlick(const Spectrum &F0)
    : _F0(F0) {
        
    }

    virtual Spectrum evaluate(Float cosI) const override {
        return schlickFresnel(_F0, cosI);
    }
    
    virtual std::string toString() const override {
        return std::string("[ FresnelSchlick etaI: ") + _F0.ToString() + std::string(" ]");
    }
    
private:
    Spectrum _F0;
};


/**
 * 菲涅尔高光
 * 其实就是镜面反射与镜面折射的组合
 * 采样时根据随机样本点去选择，采样折射还是反射
 * 具体推导过程不再赘述
 */
class FresnelSpecular : public BxDF {
public:

    FresnelSpecular(const Spectrum &R, const Spectrum &T, Float etaA,
                    Float etaB, TransportMode mode, bool thin = false)
    : BxDF(BxDFType(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_SPECULAR)),
    _R(R),
    _T(T),
    _etaA(etaA),
    _etaB(etaB),
    _mode(mode),
    _thin(thin) {
        DCHECK(!isNaN(_etaA) && !isNaN(_etaB));
    }

    virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const {
        return Spectrum(0.f);
    }

    virtual Spectrum sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                              Float *pdf, BxDFType *sampledType) const;

    virtual Float pdfDir(const Vector3f &wo, const Vector3f &wi) const { 
    	return 0; 
    }

    virtual std::string toString() const;

private:
    const Spectrum _R, _T;
    const Float _etaA, _etaB;
    const TransportMode _mode;
    const bool _thin;
};


/**
 *
 * 先简单介绍一下Oren–Nayar模型
 * Oren和Nayar观察到自然界中不存在理想的漫反射，尤其是当光照方向接近观察方向时，粗糙表面通常会显得更亮。
 * 他们建立了一个反射模型，用一个参数σ的球面高斯分布所描述的 v 形微平面来描述粗糙表面，
 * σ为微平面朝向角的标准差。
 * 在 v 形假设下，仅考虑相邻的微面即可考虑互反射;
 * 奥伦和纳亚尔利用这一点推导出了一个BRDF模型，该模型对凹槽集合的总反射进行了建模
 * 
 * 所得到的模型考虑了微观平面之间的阴影、掩蔽和相互反射，但没有解析解
 * 他们找到了一个高效又接近的方式，如下方程去近似
 * 
 * fr(wi,wo) = R/π(A + Bmax(0,cos(φi-φo))sinαtanβ)
 * 其中 A = 1 - σ^2 / (2 * σ^2 + 0.33)
 *      B = 0.45σ^2 / (σ^2 + 0.09)
 *      α = max(θi,θo)
 *      β = min(θi,θo)
 * 这个方程如何来的我也就不凑热闹了，水平有限
 *
 * 
 */
class OrenNayar : public BxDF {
public:
    OrenNayar(const Spectrum &R, Float sigma)
	: BxDF(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)), 
	_R(R) {
        sigma = degree2radian(sigma);
        Float sigma2 = sigma * sigma;
        _A = 1.f - (sigma2 / (2.f * (sigma2 + 0.33f)));
        _B = 0.45f * sigma2 / (sigma2 + 0.09f);
    }

    /**
     * 表达式如下，代码照着表达式实现就好
     * fr(wi,wo) = R/π(A + Bmax(0,cos(φi-φo))sinαtanβ)
	 * 其中 A = 1 - σ^2 / (2 * σ^2 + 0.33)
	 * 	    B = 0.45σ^2 / (σ^2 + 0.09)
	 *   	α = max(θi,θo)
	 *		β = min(θi,θo)
     */
    virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const ;
    
    virtual std::string toString() const;

    F_INLINE void setReflection(const Spectrum &R) {
        _R = R;
    }
    
private:
	// 反射系数
    Spectrum _R;
    // 表达式中的常数
    Float _A, _B;
};


PALADIN_END

#endif /* bxdf_hpp */
