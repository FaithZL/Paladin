//
//  diffuse.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/18.
//

#ifndef diffuse_hpp
#define diffuse_hpp

#include "core/light.hpp"
#include "textures/imagemap.hpp"

PALADIN_BEGIN

class DiffuseAreaLight : public AreaLight {
    
public:
    DiffuseAreaLight(const Transform *LightToWorld,
                     const MediumInterface &mediumInterface, const Spectrum &Le,
                     int nSamples, const std::shared_ptr<Shape> &shape,
                     bool twoSided = false, const string &texname = "");
    
    Spectrum L(const Interaction &intr, const Vector3f &w) const override {
        return (_twoSided || dot(intr.normal, w) > 0) ? _L : Spectrum(0.f);
    }
    
    void loadLeMap(const string &texname);
    
    virtual Spectrum sample_Le(const Point2f &u1, const Point2f &u2,
                                Float time, Ray *ray, Normal3f *nLight,
                                Float *pdfPos, Float *pdfDir) const override;
    
    virtual void pdf_Le(const Ray &ray, const Normal3f &nLight,
                        Float *pdfPos, Float *pdfDir) const override;
    
    Spectrum power() const override;
    
    virtual nloJson toJson() const override {
        return nloJson();
    }
    
    /**
     * 在场景中某处ref，采样光源，生成入射方向，
     * 得到该点的入射辐射度，并得到对应的立体角空间的pdf
     * @param  ref 场景中的点
     * @param  u   二维随机变量
     * @param  wi  生成的入射方向
     * @param  pdf 对应方向上的概率密度函数
     * @param  vis 可见测试器
     * @return     ref在wi方向上的入射辐射度
     */
    Spectrum sample_Li(const Interaction &ref, const Point2f &u, Vector3f *wo,
                       Float *pdf, VisibilityTester *vis) const override;
    
    virtual Float pdf_Li(const Interaction &, const Vector3f &) const override;
    
    virtual Float pdf_Li(const DirectSamplingRecord &rcd) const override;
    
    static shared_ptr<DiffuseAreaLight> create(Float rgb[3], const std::shared_ptr<Shape> &,
                                               const MediumInterface &mi = nullptr,
                                               const string &texname = "");
    
private:
    // 面光源才有的，发射的辐射度
    const Spectrum _L;
    std::shared_ptr<Shape> _shape;
    const bool _twoSided;
    const Float _area;
    // 面光源的辐射度
    // 要使用图片纹理作为光照辐射度
    // 必须确定的是uv映射  todo
    MIPMap<RGBSpectrum> * _Lmap;
};


DiffuseAreaLight * createDiffuseAreaLight(const nloJson &param,
                                          const std::shared_ptr<Shape> &shape,
                                          const MediumInterface &mediumInterface);

PALADIN_END

#endif /* diffuse_hpp */
