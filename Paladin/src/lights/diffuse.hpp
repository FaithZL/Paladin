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
    DiffuseAreaLight(const Transform &LightToWorld,
                     const MediumInterface &mediumInterface, const Spectrum &Le,
                     int nSamples, const std::shared_ptr<Shape> &shape,
                     bool twoSided = false);
    
    Spectrum L(const Interaction &intr, const Vector3f &w) const {
        return (_twoSided || dot(intr.normal, w) > 0) ? _L : Spectrum(0.f);
    }
    
    Spectrum power() const;
    
    Spectrum sampleLi(const Interaction &ref, const Point2f &u, Vector3f *wo,
                       Float *pdf, VisibilityTester *vis) const;
    
    Float pdfLi(const Interaction &, const Vector3f &) const;
    
private:
    // 面光源才有的，发射的辐射度
    const Spectrum _L;
    std::shared_ptr<Shape> _shape;
    const bool _twoSided;
    const Float _area;
};

PALADIN_END

#endif /* diffuse_hpp */
