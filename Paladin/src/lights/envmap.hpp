//
//  envmap.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/4.
//

#ifndef envmap_hpp
#define envmap_hpp

#include "core/light.hpp"
#include "core/scene.hpp"
#include "core/mipmap.h"

PALADIN_BEGIN

class EnvironmentMap : public Light {
    
public:
    EnvironmentMap(const Transform &LightToWorld, const Spectrum &L,
                   int nSamples, const std::string &texmap);
    
    void preprocess(const Scene &scene) {
        scene.worldBound().boundingSphere(&_worldCenter, &_worldRadius);
    }
    
    Spectrum power() const;
    
    Spectrum Le(const RayDifferential &ray) const;
    
    Spectrum sampleLi(const Interaction &ref, const Point2f &u, Vector3f *wi,
                       Float *pdf, VisibilityTester *vis) const;
    
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
    Float pdfLi(const Interaction &, const Vector3f &) const;
    
private:
    // 环境贴图的纹理
    std::unique_ptr<MIPMap<RGBSpectrum>> _Lmap;
    Point3f _worldCenter;
    Float _worldRadius;
    // 贴图对应的二维分布，用于重要性采样
    std::unique_ptr<Distribution2D> _distribution;
};

PALADIN_END

#endif /* envmap_hpp */
