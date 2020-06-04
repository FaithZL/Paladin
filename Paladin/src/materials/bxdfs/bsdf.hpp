//
//  bsdf.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/2/13.
//

#ifndef bsdf_hpp
#define bsdf_hpp

#include "core/bxdf.hpp"

PALADIN_BEGIN

/**
 * BSDF类
 * 通常物体表面都不止一种反射属性，所以需要一个类把各种BRDF，BTDF管理起来
 * 于是，就有了BSDF
 * 除了储存各种BXDF组件，还有该点微分几何信息
 *
 */
class BSDF {
public:
    BSDF(const SurfaceInteraction &si, Float eta = 1);
    
    BSDF(Float eta = 1);
    
    void updateGeometry(const SurfaceInteraction &si);

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
    Float pdfDir(const Vector3f &wo, const Vector3f &wi,
              BxDFType flags = BSDF_ALL) const;

    std::string toString() const;
    
    // 折射率，对于不透明物体，这是不用的
    const Float eta;
    
    ~BSDF() {

    }
    
    void clearBxDFs() {
        nBxDFs = 0;
    }
    
private:

    
    // 几何法线
    Normal3f _gNormal;
    // 着色法线，bump贴图，法线贴图之类的
    Normal3f _sNormal;
    // 着色切线(s方向，u方向)
    Vector3f _sTangent;
    // 着色切线(t方向，v方向)
    Vector3f _tTangent;
    // BXDF组件的数量
    int nBxDFs = 0;
    // BXDF组件的最大数量
    static CONSTEXPR int MaxBxDFs = 8;
    // BXDF列表
    BxDF *bxdfs[MaxBxDFs];

    friend class MixMaterial;
};

PALADIN_END

#endif /* bsdf_hpp */
