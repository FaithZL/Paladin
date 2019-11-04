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
    EnvironmentMap(const Transform &LightToWorld, const Spectrum &power,
                   int nSamples, const std::string &texmap);
    
    void preprocess(const Scene &scene) {
        scene.worldBound().boundingSphere(&_worldCenter, &_worldRadius);
    }
    
    Spectrum power() const;
    
    Spectrum Le(const RayDifferential &ray) const;
    
    Spectrum sampleLi(const Interaction &ref, const Point2f &u, Vector3f *wi,
                       Float *pdf, VisibilityTester *vis) const;
    
    Float pdfLi(const Interaction &, const Vector3f &) const;
    
private:
    // 环境贴图的纹理
    std::unique_ptr<MIPMap<RGBSpectrum>> _Lmap;
    Point3f _worldCenter;
    Float _worldRadius;
    std::unique_ptr<Distribution2D> _distribution;
};

PALADIN_END

#endif /* envmap_hpp */
