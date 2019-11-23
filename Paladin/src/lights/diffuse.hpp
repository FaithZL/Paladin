//
//  diffuse.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/18.
//

#ifndef diffuse_hpp
#define diffuse_hpp

#include "core/light.hpp"

PALADIN_BEGIN

class DiffuseAreaLight : public AreaLight {
    
public:
    DiffuseAreaLight(const Transform * LightToWorld,
                     const MediumInterface &mediumInterface, const Spectrum &Le,
                     int nSamples, const std::shared_ptr<Shape> &shape,
                     bool twoSided = false);
    
    Spectrum L(const Interaction &intr, const Vector3f &w) const override {
        return (_twoSided || dot(intr.normal, w) > 0) ? _L : Spectrum(0.f);
    }
    
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
    
    Float pdf_Li(const Interaction &, const Vector3f &) const override;
    
private:
    // 面光源才有的，发射的辐射度
    const Spectrum _L;
    std::shared_ptr<Shape> _shape;
    const bool _twoSided;
    const Float _area;
};

DiffuseAreaLight * createDiffuseAreaLight(const Transform * LightToWorld,
                                          const MediumInterface &mediumInterface, const Spectrum &Le,
                                          int nSamples, const std::shared_ptr<Shape> &shape,
                                          bool twoSided = false);

DiffuseAreaLight * createDiffuse(const nloJson &param,
                                const Transform * lightToWorld,
                                const std::shared_ptr<Shape> &shape);

PALADIN_END

#endif /* diffuse_hpp */
