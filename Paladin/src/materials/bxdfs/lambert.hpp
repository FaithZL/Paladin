//
//  lambert.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/2/12.
//

#ifndef lambert_hpp
#define lambert_hpp

#include "core/bxdf.hpp"

PALADIN_BEGIN

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
    
    PALADIN_INLINE void setReflection(const Spectrum &R) {
        _R = R;
    }

private:
    // 反射系数
    Spectrum _R;
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

    virtual Float pdfDir(const Vector3f &wo, const Vector3f &wi) const;

    virtual std::string toString() const;

private:
      // 透射系数
    Spectrum _T;
};



PALADIN_END

#endif /* lambert_hpp */
