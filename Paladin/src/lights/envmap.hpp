//
//  envmap.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/4.
//

#ifndef envmap_hpp
#define envmap_hpp

#include "core/light.hpp"
#include "core/mipmap.h"
#include "math/sampling.hpp"

PALADIN_BEGIN

class EnvironmentMap : public Light {
    
public:
    EnvironmentMap(const Transform *LightToWorld, const Spectrum &L,
                   int nSamples, const std::string &texmap);
    
    virtual void preprocess(const Scene &scene) override;
    
    Spectrum power() const override;
    
    Spectrum Le(const RayDifferential &ray) const override;
    
    Spectrum Le(const RayDifferential &ray, Float *pdf) const override;
    
    F_INLINE Spectrum evalEnvironment(const RayDifferential &ray, Float *pdf) const {
        Vector3f wi = normalize(_worldToLight->exec(ray.dir));
        Point2f st(sphericalPhi(wi) * Inv2Pi, sphericalTheta(wi) * InvPi);
        *pdf = _distribution->pdf(st);
        Float theta = st[1] * Pi;
        Float sinTheta = std::sin(theta);
        *pdf = sinTheta == 0 ? 0 : *pdf / (2 * Pi * Pi * sinTheta);
        return *pdf == 0 ? 0 : Spectrum(_Lmap->lookup(st), SpectrumType::Illuminant);
    }
    
    virtual nloJson toJson() const override {
        return nloJson();
    }
    
    virtual Spectrum sample_Le(const Point2f &u1, const Point2f &u2,
                                Float time, Ray *ray, Normal3f *nLight,
                                Float *pdfPos, Float *pdfDir) const override;
    
    virtual void pdf_Le(const Ray &ray, const Normal3f &nLight,
                        Float *pdfPos, Float *pdfDir) const override;
    
    virtual Spectrum sample_Li(const Interaction &ref, const Point2f &u, Vector3f *wi,
                       Float *pdf, VisibilityTester *vis) const override;
    
    virtual Spectrum sample_Li(DirectSamplingRecord *rcd, const Point2f &u,
                               const Scene &) const override;
    
    /**
     * 求基于方向的pdf，要把uv空间的pdf转为立体角空间的pdf
     * 推导过程如下所示
     * 
     * u = φ / 2π
     * v = θ / π
     * p(u, v) du dv = p(ω) dω
     * 
     * p(u, v) / p(ω) = dω / dudv = (sinθ dθ dφ) / dudv
     * 
     * p(u, v) / p(ω) = sinθ 2π^2
     * 
     */
    virtual Float pdf_Li(const Interaction &, const Vector3f &) const override;
    
    /**
     * 求基于方向的pdf，要把uv空间的pdf转为立体角空间的pdf
     * 推导过程如下所示
     *
     * u = φ / 2π
     * v = θ / π
     * p(u, v) du dv = p(ω) dω
     *
     * p(u, v) / p(ω) = dω / dudv = (sinθ dθ dφ) / dudv
     *
     * p(u, v) / p(ω) = sinθ 2π^2
     *
     */
    virtual Float pdf_Li(const DirectSamplingRecord &) const override;
    
private:
    // 环境贴图的纹理
    std::unique_ptr<MIPMap<RGBSpectrum>> _Lmap;
    Point3f _worldCenter;
    Float _worldRadius;
    // 贴图对应的二维分布，用于重要性采样
    std::unique_ptr<Distribution2D> _distribution;
};

CObject_ptr createEnvironmentMap(const nloJson &param, const Arguments &lst);

PALADIN_END

#endif /* envmap_hpp */
