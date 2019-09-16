//
//  bxdf.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef bxdf_hpp
#define bxdf_hpp

#include "core/header.h"
#include "core/spectrum.hpp"

PALADIN_BEGIN

/**
 * 描述如何反射或折射的模块
 * -----------------------------------------------------------
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
 * 
 */

// 以下函数都默认一个条件，w为单位向量
// 比较简单，就不写推导过程了
inline Float cosTheta(const Vector3f &w) {
    return w.z;
}

inline Float cos2Theta(const Vector3f &w) {
    return w.z * w.z;
}

inline Float absCosTheta(const Vector3f &w) {
    return std::abs(w.z);
}

inline Float sin2Theta(const Vector3f &w) {
    return std::max((Float)0, (Float)1 - cos2Theta(w));
}

inline Float sinTheta(const Vector3f &w) { 
    return std::sqrt(sin2Theta(w)); 
}

inline Float tanTheta(const Vector3f &w) { 
    return sinTheta(w) / cosTheta(w); 
}

inline Float tan2Theta(const Vector3f &w) {
    return sin2Theta(w) / cos2Theta(w);
}

inline Float cosPhi(const Vector3f &w) {
    Float _sinTheta = sinTheta(w);
    return (_sinTheta == 0) ? 1 : clamp(w.x / _sinTheta, -1, 1);
}

inline Float sinPhi(const Vector3f &w) {
    Float _sinTheta = sinTheta(w);
    return (_sinTheta == 0) ? 0 : clamp(w.y / _sinTheta, -1, 1);
}

inline Float cos2Phi(const Vector3f &w) { 
    return cosPhi(w) * cosPhi(w);
}

inline Float sin2Phi(const Vector3f &w) { 
    return sinPhi(w) * sinPhi(w);
}

inline Float cosDPhi(const Vector3f &wa, const Vector3f &wb) {
    return clamp(
                 (wa.x * wb.x + wa.y * wb.y) / std::sqrt((wa.x * wa.x + wa.y * wa.y) *
                                                         (wb.x * wb.x + wb.y * wb.y)),
                 -1, 1);
}

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
Float frDielectric(Float cosThetaI, Float etaI, Float etaT);

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
Spectrum frConductor(Float cosThetaI, const Spectrum &etaI,
                     const Spectrum &etaT, const Spectrum &kt);

/**
 * 返回的是入射光线的反方向
 * 简单说一下推导过程，注释画图不方便   
 * 这里隐含的假设是n与wo都是单位向量
 *
 * 在草稿纸上画图，wi + wo = OC，OC为wi与wo角平分线，与法线n方向一致
 * θ为OC与wo夹角
 * OC/2 = |wo| * cosθ * n/|n|
 * cosθ = dot(wo,n)/|n||wo|
 *
 * wi = OC - wi = 2(dot(wo,n) * n/|n|) - wi
 * 
 * 又因为n为单位向量
 *
 * 则wi = 2(dot(wo,n) * n) - wi
 * 
 * @param  wo 出射光线方向，单位向量
 * @param  n  法线方向，单位向量
 * @return    注意这里返回的是入射光线的反方向wi
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

enum BxDFType {
    BSDF_REFLECTION = 1 << 0,
    BSDF_TRANSMISSION = 1 << 1,
    BSDF_DIFFUSE = 1 << 2,
    BSDF_GLOSSY = 1 << 3,
    BSDF_SPECULAR = 1 << 4,
    BSDF_ALL = BSDF_DIFFUSE
            | BSDF_GLOSSY
            | BSDF_SPECULAR
            | BSDF_REFLECTION
            | BSDF_TRANSMISSION,
};

class BSDF {
    
public:
    
private:
    ~BSDF() {}
    
    const Normal3f ns, ng;
    const Vector3f ss, ts;
    int nBxDFs = 0;
    static CONSTEXPR int MaxBxDFs = 8;
    BxDF *bxdfs[MaxBxDFs];
    friend class MixMaterial;
};

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
    
    bool MatchesFlags(BxDFType t) const {
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
    
    /**
     * 指定出射方向，根据随机样本点采样入射方向
     * 返回radiance，同时通过指针返回入射方向以及该方向的概率密度函数
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
    virtual Float pdfW(const Vector3f &wo, const Vector3f &wi) const;
    
    virtual std::string toString() const = 0;
    
    const BxDFType type;
};

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

class Fresnel {
public:
    virtual ~Fresnel() {
        
    }
    
    virtual Spectrum evaluate(Float cosI) const = 0;
    
    virtual std::string toString() const = 0;
};

inline std::ostream &operator<<(std::ostream &os, const Fresnel &f) {
    os << f.toString();
    return os;
}

class FresnelConductor : public Fresnel {
public:
    Spectrum Evaluate(Float cosThetaI) const;
    
    FresnelConductor(const Spectrum &etaI, const Spectrum &etaT,
                     const Spectrum &k)
    : _etaI(etaI), _etaT(etaT), _k(k) {
        
    }
    
    std::string toString() const;
    
private:
    Spectrum _etaI, _etaT, _k;
};

class FresnelDielectric : public Fresnel {
public:
    virtual Spectrum evaluate(Float cosThetaI) const;
    
    FresnelDielectric(Float etaI, Float etaT) : _etaI(etaI), _etaT(etaT) {
        
    }
    
    virtual std::string toString() const;
    
private:
    Float _etaI, _etaT;
};

PALADIN_END

#endif /* bxdf_hpp */
