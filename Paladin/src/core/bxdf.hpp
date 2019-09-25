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
#include "sampling.hpp"
#include "microfacet.hpp"

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

/**
 * 反射类型
 * 一个bxdf的类型至少要有一个BSDF_REFLECTION 或 BSDF_TRANSMISSION
 * 用于表明是投射还是反射
 */
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
    
    /**
     * 这个函数可以理解为:
     * 该BXDF函数的蒙特卡洛估计的一个样本值
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

inline std::ostream &operator<<(std::ostream &os, const BxDF &f) {
    os << f.toString();
    return os;
}

/**
 * BSDF类
 * 通常物体表面都不止一种反射属性，所以需要一个类把各种BRDF，BTDF管理起来
 * 于是，就有了BSDF
 * 除了储存各种BXDF组件，还有该点微分几何信息
 * 
 */
class BSDF {
public:
    BSDF(const SurfaceInteraction &si, Float eta = 1)
    : eta(eta),
    _sNormal(si.shading.normal),
    _gNormal(si.normal),
    _sTangent(normalize(si.shading.dpdu)),
    _tTangent(cross(_sNormal, _sTangent)) {

    }

    void add(BxDF *b) {
        CHECK_LT(nBxDFs, MaxBxDFs);
        bxdfs[nBxDFs++] = b;
    }

    int numComponents(BxDFType flags = BSDF_ALL) const {
        int num = 0;
        for (int i = 0; i < nBxDFs; ++i) {
            if (bxdfs[i]->matchesFlags(flags)) ++num;
        }
        return num;
    }

    /**
     * 根据法线方向，切线向量，付切线向量确定正交基 
     * @param  v [description]
     * @return   [description]
     */
    Vector3f worldToLocal(const Vector3f &v) const {
        return Vector3f(dot(v, _sTangent), dot(v, _tTangent), dot(v, _sNormal));
    }

    Vector3f localToWorld(const Vector3f &v) const {
        return Vector3f(_sTangent.x * v.x + _tTangent.x * v.y + _sNormal.x * v.z,
                        _sTangent.y * v.x + _tTangent.y * v.y + _sNormal.y * v.z,
                        _sTangent.z * v.x + _tTangent.z * v.y + _sNormal.z * v.z);
    }

    Spectrum f(const Vector3f &woW, const Vector3f &wiW,
               BxDFType flags = BSDF_ALL) const;

    /**
     * 跟BXDF的rho_hh函数相同，不再赘述
     */
    Spectrum rho_hh(int nSamples, const Point2f *samples1, const Point2f *samples2,
                 BxDFType flags = BSDF_ALL) const;

    /**
     * 跟BXDF的rho_hd函数相同，不再赘述
     */    
    Spectrum rho_hd(const Vector3f &wo, int nSamples, const Point2f *samples,
                 BxDFType flags = BSDF_ALL) const;

    Spectrum sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                      Float *pdf, BxDFType type = BSDF_ALL,
                      BxDFType *sampledType = nullptr) const;
    
    /**
     * 跟BXDF的pdfW函数相同，不再赘述
     */ 
    Float pdfW(const Vector3f &wo, const Vector3f &wi,
              BxDFType flags = BSDF_ALL) const;

    std::string toString() const;
    
    // 折射率，对于不透明物体，这是不用的
    const Float eta;
    
private:

    ~BSDF() {

    }
    // 几何法线
    const Normal3f _gNormal;
    // 着色法线，bump贴图，法线贴图之类的
    const Normal3f _sNormal;
    // 着色切线(s方向，u方向)
    const Vector3f _sTangent;
    // 着色切线(t方向，v方向)
    const Vector3f _tTangent;
    // BXDF组件的数量
    int nBxDFs = 0;
    // BXDF组件的最大数量
    static CONSTEXPR int MaxBxDFs = 8;
    // BXDF列表
    BxDF *bxdfs[MaxBxDFs];

    friend class MixMaterial;
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
        return frConductor(std::abs(cosThetaI), _etaI, _etaT, _kt);
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
        return frDielectric(cosThetaI, _etaI, _etaT);
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
};

/**
 * 理想镜面反射
 * 狄拉克delta分布
 */
class SpecularReflection : public BxDF {
    
public:
    SpecularReflection(const Spectrum &R, Fresnel *fresnel)
    : BxDF(BxDFType(BSDF_REFLECTION | BSDF_SPECULAR)),
    _R(R),
    _fresnel(fresnel) {
        
    }
    
    /**
     * 由于是理想镜面反射，狄拉克函数
     * 用常规方式无法采样，需要特殊处理
     * @param  wo 出射方向
     * @param  wi 入射方向
     * @return    0
     */
    virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const {
        return Spectrum(0.f);
    }
    
    /**
     * 理想镜面反射的采样函数，是固定方向采样，首先要计算合适的入射方向
     * Lo的蒙特卡洛估计表达式如下
     * 		      1	             fr(p,wo,wi)Li(p,wi)|cosθi|
     * Lo(wo) = ----- * Σ[i,N] ------------------------------
     * 		      N 					  p(wi)
     * 
     * 		      1	             (ρhd(wo)δ(wi-wr)/|cosθi|)Li(p,wi)|cosθi|
     * Lo(wo) = ----- * Σ[i,N] --------------------------------------------
     * 		      N 					  		  p(wi) 
     * 
     * p(wi)为狄拉克函数，p(wi) = δ(wi-wr)，两个狄拉克函数可以抵消，
     * 
     * 最终得到Lo(wo) = ρhd(wo) * Li(p, wr)
     * 由于是非常规采样，pdf为1
     * 
     *                 δ(wi - wr)Fr(wi)
     * fr(p,wo,wi) = -----------------------   3式
     *                     |cosθr|
     *
     * ρhd(wo) = ∫[hemisphere]fr(p,wi,wo)|cosθi|dwi 4式 (ρhd(wo)的定义)
     *                     
     * 联合3,4式可得如下代码
     *                     
     * @param  wo          出射方向
     * @param  wi          入射方向
     * @param  sample      在此函数中不需要使用的样本点
     * @param  pdf         概率密度函数值为1
     * @param  sampledType 不需要使用的采样类型
     * @return             [description]
     */
    virtual Spectrum sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample,
                              Float *pdf, BxDFType *sampledType) const ;
    /**
     * 由于是理想镜面反射，狄拉克函数
     * 用常规方式无法采样，需要特殊处理
     * @param  wo 出射方向
     * @param  wi 入射方向
     * @return    0
     */
    virtual Float pdfW(const Vector3f &wo, const Vector3f &wi) const {
        return 0;
    }
    
    virtual std::string toString() const;
    
private:
	// 用于缩放颜色频谱
    const Spectrum _R;
    // 菲涅尔对象指针
    const Fresnel *_fresnel;
};

/**
 * 理想镜面透射
 * 用于建立光线照射在水面上的数学模型
 * 
 * 先来简单推导一下所用到的表达式
 * 想象一下光线照射在水面上，一部分光直接反射，另一部分光射入水中，
 * 反射光的radiance我们可以直接通过菲涅尔函数计算出来
 * 折射光的radiance就没那么简单，因为在两个介质交界处，光线的微分立体角会发生变化(因为折射率不同)
 * 
 * 根据能量守恒定律，微分立体角发生变化，radiance也就发生了变化
 * 我们用τ表示被折射的能量的比率则τ = 1 - Fr(wi)
 * Φ 表示辐射通量
 * 
 * dΦo = τ * dΦi
 * 将radiance带入，得
 * 
 * Lo * cosθo * dA * dωo = τ(Li * cosθi * dA * dωi)
 * 
 * 将微分立体角展开，得
 * 
 * Lo * cosθo * dA * sinθodθodφo = τ * Li * cosθi * dA * sinθidθidφi   2式
 * 
 * 对斯涅尔定律ηi * sinθi = ηo * sinθo 求导，得
 * 
 * ηo * cosθodθo = ηi * cosθidθi  3式
 *
 * 联合2，3式，得
 * 
 * Lo * ηi^2 * dφo = τLi * ηo^2 * dφi
 *
 * 整理之后，如下
 * 
 * Lo = τ * Li * (ηo/ηi)^2
 *
 * BTDF的表达式如下
 *                ηo                          δ(wi - T(wo,n))
 * fr(wo, wi) = (----)^2 * (1 - Fr(wi)) * ----------------------     (T为折射函数，n为法线)
 *                ηi                            |cosθi|
 * 
 */
class SpecularTransmission : public BxDF {
public:
    SpecularTransmission(const Spectrum &T, Float etaA, Float etaB,
                         TransportMode mode)
    : BxDF(BxDFType(BSDF_TRANSMISSION | BSDF_SPECULAR)),
    _T(T),
    _etaA(etaA),
    _etaB(etaB),
    _fresnel(etaA, etaB),
    _mode(mode) {
        
    }
    
    virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const {
        return Spectrum(0.f);
    }
    
    /**
     * 采样理想镜面透射
     * BTDF的表达式如下
     *                ηo                          δ(wi - T(wo,n))
     * fr(wo, wi) = (----)^2 * (1 - Fr(wi)) * ----------------------
     *                ηi                            |cosθi|
     * 
     * 跟镜面反射一样，无法用常规方式采样处理
     * pdf直接赋值为1，原理与上面的理想镜面反射相同，不再赘述
     * 
     */
    virtual Spectrum sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample,
                              Float *pdf, BxDFType *sampledType) const;
    
    virtual Float pdfW(const Vector3f &wo, const Vector3f &wi) const {
        return 0; 
    }
    
    virtual std::string toString() const;
    
private:
	// 用于缩放颜色频谱
    const Spectrum _T;

    // _etaA是表面以上介质的折射率 (above)
    // _etaB是物体表面以下介质的折射率 (below)
    const Float _etaA, _etaB;
    const FresnelDielectric _fresnel;
    // 记录传输模式(从光源发出or从相机发出)
    const TransportMode _mode;
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
                    Float etaB, TransportMode mode)
    : BxDF(BxDFType(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_SPECULAR)),
      _R(R),
      _T(T),
      _etaA(etaA),
      _etaB(etaB),
      _mode(mode) {

    }

    virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const {
        return Spectrum(0.f);
    }

    virtual Spectrum sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                              Float *pdf, BxDFType *sampledType) const;

    virtual Float pdfW(const Vector3f &wo, const Vector3f &wi) const { 
    	return 0; 
    }

    virtual std::string toString() const;

private:
    const Spectrum _R, _T;
    const Float _etaA, _etaB;
    const TransportMode _mode;
};


/**
 * 朗伯反射又称理想漫反射
 * 半空间上各个方向的反射率相同
 * 理想漫反射在物理上是不可能的
 * 
 * 接下来推导一下朗伯反射的brdf函数
 *
 * 由能量守恒得 ∫fr(p,wo,wi)|cosθi|dwi = 1
 *
 * 朗伯反射各个方向的brdf函数值相等，假设fr(p,wo,wi) = k，
 * 又因为入射角不大于π/2，所以cosθi不小于零，得
 *
 * ∫kcosθidwi = 1
 *
 * 由立体角定义展开dwi，得
 *
 * ∫[0,2π]∫[0,π/2]kcosθsinθdθdφ = 1
 * 
 * 移项，得
 * 
 * ∫[0,2π]∫[0,π/2]cosθsinθdθdφ = 1/k
 * 
 * 非常非常简单的定积分计算，求得等号左边的值为π
 * 
 * 所以 k = 1/π
 */
class LambertianReflection : public BxDF {
public:
    LambertianReflection(const Spectrum &R)
    : BxDF(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)),
    _R(R) {

	}

	// 朗伯反射中任何方向的反射率都相等
    virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const {
		return _R * InvPi;
    }

    // 朗伯反射中任何方向的反射率都相等
    virtual Spectrum rho_hd(const Vector3f &, int, const Point2f *) const { 
    	return _R; 
    }

    // 朗伯反射中任何方向的反射率都相等
    virtual Spectrum rho_hh(int, const Point2f *, const Point2f *) const { 
    	return _R; 
    }

    virtual std::string toString() const {
    	return std::string("[ LambertianReflection R: ") + _R.ToString() +
           std::string(" ]");
    }

private:
	// 反射系数
    const Spectrum _R;
};


/**
 * 朗伯透射
 * 原理跟朗伯反射相同，具体推导过程参见朗伯反射
 * 不再赘述
 */
class LambertianTransmission : public BxDF {
public:
    LambertianTransmission(const Spectrum &T)
    : BxDF(BxDFType(BSDF_TRANSMISSION | BSDF_DIFFUSE)), 
    _T(T) {

    }

    virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const {
    	return _T * InvPi;
    }

    virtual Spectrum rho_hd(const Vector3f &, int, const Point2f *) const { 
    	return _T; 
    }

    virtual Spectrum rho_hh(int, const Point2f *, const Point2f *) const { 
    	return _T; 
    }

    virtual Spectrum sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                              Float *pdf, BxDFType *sampledType) const;

    virtual Float pdfW(const Vector3f &wo, const Vector3f &wi) const;

    virtual std::string toString() const;

private:
  	// 透射系数
    Spectrum _T;
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

private:
	// 反射系数
    const Spectrum _R;
    // 表达式中的常数
    Float _A, _B;
};

/**
 * 
 * 微面元反射DRDF
 * 关于微面元的法线分布函数介绍在microfacet.hpp文件中
 * 一个较早的微平面模型在1967年 Torrance 和 Sparrow用于模拟金属表面
 * 假设一个宏观平面有若干个微平面构成，微平面的法向量为ωh
 * ωh与ωo的夹角为θh   ωh与ωi夹角为θh
 * 观察光线出射方向为ωo，入射方向为ωi则满足，
 * 
 *     ωh = ωo + ωi   1式
 *      
 * 现在我们来推导一下微面元反射的BRDF表达式
 *
 * 先来看看h方向上的微分辐射通量的表达式，根据flux与radiance的关系，可得
 * 
 *      dΦh = Li(ωi) dω dA⊥(ωh) = Li(ωi) dω cosθh dA(ωh)    2式
 *
 * ωh方向上的微平面面积如下
 *
 *      dA(ωh) = D(ωh) dωh dA       3式      (由法线分布函数的定义可得)
 *
 * 将3式带入2式
 *
 *      dΦh = Li(ωi) dω cosθh D(ωh) dωh dA     4式
 *      
 * 我们假设各个微平面根据菲涅尔定律独立反射光线，则出射辐射通量flux为
 *
 *      dΦo = Fr(ωo) dΦh   5式    (这里理解得不是很好，菲涅尔函数表示的就是有多少能量没被吸收，直接反射出来了)
 *
 * 再由radiance的定义
 *                     dΦo
 *      Lo(ωo) = ------------------     6式
 *                 cosθo dωo dA
 *
 * 联合4，5，6式，可得
 *
 *                  Fr(ωo) Li(ωi) dωi D(ωh) dωh dA cosθh 
 *      Lo(ωo) = ---------------------------------------------          7式
 *                              cosθo dωo dA
 *
 * 这里直接使用一个结论表达式(在pdfW方法的注释中给出推导过程)
 *
 *      dωo = 4 cosθh dωh       8式
 *
 * 联合7，8式
 *                 Fr(ωo) Li(ωi) D(ωh) dωi 
 *      Lo(ωo) = -----------------------------           9式
 *                       4 cosθo
 *
 *                             dLo(p, ωo)               dLo(p, ωo) 
 * 由BRDF定义 fr(p, ωo, ωi) = ------------------ = ------------------------   10式
 *                              dE(p, ωi)           Li(p, ωi) cosθi dωi
 *
 * 10式定义了fr，用人话来说就是fr(p, ωo, ωi)表示的是：
 * ωi方向辐照度(irradiance)变化时，ωo方向的辐射度(radiance)的变化率
 * 
 * 联合9，10两式，再加入几何遮挡项
 *
 *                   D(ωh) G(ωi,ωo) Fr(ωo)
 * fr(p, ωo, ωi) = ------------------------     11式
 *                     4 cosθo cosθi
 *
 * 到此，BRDF就这样推导完毕
 *
 * 顺便说一下，为何BRDF的定义为辐射度比辐照度，量纲为1/sr（sr为立体角）
 * 说一下我的个人理解
 *
 * 我们观察到一个物体的表面的点，被无数个方向的光照射到，然后向我们眼睛的方向反射了一部分光
 * 所以我们才能看到该点
 *
 * 我们需要定义一个分布函数，用来表示每个入射方向的光线的贡献大小
 * 
 * Lo(wo) = ∫[hemisphere]fr(ωo,ωi)Li(ωi)|cosθi|dωi      12式
 *
 * 12式表达了半空间上所有方向的入射辐射度乘以fr之后再相加，得到的是出射辐射度
 *
 * 由以上表达式，自然可以得出一个结论fr的量纲为1/sr（sr为立体角）
 * 感觉自己说得也不清晰
 *
 */
class MicrofacetReflection : public BxDF {
public:
    MicrofacetReflection(const Spectrum &R,
                         MicrofacetDistribution *distribution, Fresnel *fresnel)
    : BxDF(BxDFType(BSDF_REFLECTION | BSDF_GLOSSY)),
    _R(R),
    _distribution(distribution),
    _fresnel(fresnel) {
        
    }
    
    /**
     * 函数值，原始表达式如下
     *                   D(ωh) G(ωi,ωo) Fr(ωo)
     * fr(p, ωo, ωi) = ------------------------   
     *                     4 cosθo cosθi
     * @param  wo 出射方向
     * @param  wi 入射方向
     * @return    [description]
     */
    Spectrum f(const Vector3f &wo, const Vector3f &wi) const;
    
    /**
     * 函数的样本值，并返回该样本的概率密度函数值
     * @param  wo          出射方向
     * @param  wi          输出的入射方向
     * @param  u           决定入射方向的样本点
     * @param  pdf         输出的概率密度函数值
     * @param  sampledType 采样类型
     * @return             [description]
     */
    Spectrum sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                      Float *pdf, BxDFType *sampledType) const;
    
    /**
     * 采样wi概率密度函数，
     * 在MicrofacetDistribution中我们只实现了wh的概率密度函数
     * 这里要将wh的分布转换到wi的分布
     * θi为ωi与ωo的夹角，θh为ωh与ωo的夹角，ωo方向不变，ωh的改变会引起ωi的变换，
     * 知道以上信息之后，可以开始推导了
     *
     *      θi = 2θh,  dφi = dφh
     *      
     *                 sinθh dθh dφh
     * dωh / dωi = -------------------- 
     *                 sinθi dθi dφi
     *
     *                 sinθh dθh            sinθh           1
     * dωh / dωi = -------------------- = ---------- = ---------- = dωh / dωo
     *                sin2θh d2θh          2sin2θh       4cosθh
     * 
     * 又由sampling.hpp 中 1 式 py(y) * dy/dx = px(x) 可得
     *
     * pωi(ωi) = dωh / dωi * pωh(ωh) = pωh(ωh) / 4cosθh
     *                 
     * @param  wo 出射方向
     * @param  wi 入射方向
     * @return    [description]
     */
    Float pdfW(const Vector3f &wo, const Vector3f &wi) const;
    
    std::string toString() const;
    
private:
    // 反射率
    const Spectrum _R;
    // 微平面分布
    const MicrofacetDistribution *_distribution;
    // 菲涅尔
    const Fresnel *_fresnel;
};

/**
 * 微面元透射 BTDF MicrofacetTransmission
 * 我们在实现 MicrofacetReflection类的时候已经推导过dωo与dωh的关系式
 *           dωo
 * dωh = ----------
 *         4cosθh
 * 但在 MicrofacetTransmission中，这两者的关系式是不同的
 * 关系式如下
 *                 ηo^2 |ωo · ωh| dωo
 *    dωh = --------------------------------            1式   (这个没有手动推过，比较羞耻，todo，有空一定要搞搞)
 *            [ηi(ωh · ωi) + ηo(ωo · ωh)]^2
 *
 * 补上1式推导的思路，我们需要求的是dωh/dωo，实际上可以把ωi指定为基准向量(θ为零)，ωi向量方向不变
 * 我们可以计算ωh向量变化时，对应ωo的变化率，实际上就是dωh/dωo的倒数，则
 * θi为入射角，θo为折射角
 * 又因为ωi指定为基准向量，ωh与ωi的夹角 = π + θi - θo，ωo与ωi的夹角 = π - θo
 * 由立体角定义展开如下
 *
 *              sin(π + θi - θo)d(π + θi - θo)dφh
 * dωh/dωo = ------------------------------------    (φh = φo，可以约分)
 *                   sin(π - θo)d(π - θo)dφo
 *
 * 上述有两个未知量θi，θo，又由斯涅尔定律  ηi * sinθi = ηo * sinθo
 * 用斯涅尔定律表达式带入dωh/dωo表达式，各种换元法，求导。。。。。。应该就可以推导出1式了
 * (计算量太大了，暂未尝试，先欠着)
 *
 *
 *
 *                 (1 - Fr(ωo)) Li(ωi) dωi D(ωh) dωh dA cosθh 
 *    Lo(ωo) = ------------------------------------------------   2式 (MicrofacetReflection中的7式修改而来)
 *                          cosθo dωo dA
 *
 *                             dLo(p, ωo)               dLo(p, ωo) 
 * 由BTDF定义 fr(p, ωo, ωi) = ------------------ = ------------------------   3式(MicrofacetReflection中的10式)
 *                              dE(p, ωi)           Li(p, ωi) cosθi dωi
 *                          
 * 联合1，2，3式，再加入几何遮挡函数
 *
 *                 η^2 (1 - Fr(ωo)) D(ωh) G(ωi,ωo)   |ωi · ωh||ωo · ωh|
 * fr(ωo, ωi) = ---------------------------------------------------------
 *                   [(ωh · ωo) + η(ωi · ωh)]^2   cosθo cosθi
 *                   
 * 其中 η = ηi/ηo , ωh = ωo + ηωi
 * 
 */
class MicrofacetTransmission : public BxDF {
public:
    MicrofacetTransmission(const Spectrum &T,
                           MicrofacetDistribution *distribution, Float etaA,
                           Float etaB, TransportMode mode)
    : BxDF(BxDFType(BSDF_TRANSMISSION | BSDF_GLOSSY)),
    _T(T),
    _distribution(distribution),
    _etaA(etaA),
    _etaB(etaB),
    _fresnel(etaA, etaB),
    _mode(mode) {
        
    }
    
    /**
     * BTDF 函数值
     *                η^2 (1 - Fr(ωo)) D(ωh) G(ωi,ωo)   |ωi · ωh||ωo · ωh|
     * fr(ωo, ωi) = ---------------------------------------------------------
     *                   [(ωh · ωo) + η(ωi · ωh)]^2    cosθo cosθi
     * @param  wo 出射向量
     * @param  wi 入射向量
     * @return    BTDF函数值
     */
    virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const;
    
    virtual Spectrum sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                              Float *pdf, BxDFType *sampledType) const;
    
    /**
     * dωh 与 dωi的关系式
     *                   ηi^2 |ωi · ωh| 
     * dωh/dωi = --------------------------------    (推导思路与dωh/dωo类似)  
     *            [ηo(ωh · ωo) + ηi(ωi · ωh)]^2       
     *
     *  又由sampling.hpp 中 1 式 py(y) * dy/dx = px(x) 可得
     *
     * pωi(ωi) = dωh / dωi * pωh(ωh)
     *            
     * @param  wo [description]
     * @param  wi [description]
     * @return    [description]
     */
    virtual Float pdfW(const Vector3f &wo, const Vector3f &wi) const;
    
    virtual std::string toString() const;
    
private:

    const Spectrum _T;
    const MicrofacetDistribution *_distribution;
    const Float _etaA, _etaB;
    const FresnelDielectric _fresnel;
    const TransportMode _mode;
};

/**
 * 菲涅尔混合
 * 用于模拟漫反射物体表面有一层光泽反射层
 *
 *                            D(ωh) F(ωo)
 * fr(p, ωo, ωi) = ------------------------------------
 *                   4(ωh · ωi) * max((n,ωi), (n,ωo))
 *
 * 根据BRDF互换性，能量守恒
 * 
 * Fr(cosθ) = R +(1 - R)(1 - cosθ)^5   (这个表达式是近似)
 *
 *
 *                  28 Rd
 * fr(p, ωo, ωi) = ------- (1 - Rs) (1 - (1 - (n · ωi)/2)^5) (1 - (1 - (n · ωo)/2)^5)
 *                   23π  
 *
 * 第一个表达式待推导一遍todo，后面两个还是暂时认怂吧
 * 
 */
class FresnelBlend : public BxDF {
public:
    FresnelBlend(const Spectrum &Rd, const Spectrum &Rs,
                 MicrofacetDistribution *distrib)
    : BxDF(BxDFType(BSDF_REFLECTION | BSDF_GLOSSY)),
    _Rd(Rd),
    _Rs(Rs),
    _distribution(distrib) {
        
    }

    /**
     *                  28 Rd
     * fr(p, ωo, ωi) = ------- (1 - Rs) (1 - (1 - (n · ωi)/2)^5) (1 - (1 - (n · ωo)/2)^5)
     *                   23π  
     * @param  wo 出射方向
     * @param  wi 入射方向
     */
    virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const;
    
    /**
     * Fr(cosθ) = R +(1 - R)(1 - cosθ)^5
     */
    Spectrum schlickFresnel(Float _cosTheta) const;
    
    /**
     * 这个函数的分布转换需要再搞搞，todo
     */
    virtual Spectrum sample_f(const Vector3f &wo, Vector3f *wi,
                              const Point2f &uOrig, Float *pdf,
                              BxDFType *sampledType) const;

    virtual Float pdfW(const Vector3f &wo, const Vector3f &wi) const;
    
    virtual std::string toString() const;
    
private:
    const Spectrum _Rd, _Rs;
    MicrofacetDistribution *_distribution;
};

PALADIN_END

#endif /* bxdf_hpp */
