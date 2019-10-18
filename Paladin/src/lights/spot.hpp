//
//  spot.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/18.
//

#ifndef spot_hpp
#define spot_hpp

#include "core/light.hpp"

PALADIN_BEGIN

class SpotLight : public Light {
    
public:
    SpotLight(const Transform &LightToWorld, const MediumInterface &m,
              const Spectrum &I, Float totalWidth, Float falloffStart);
    
    virtual Spectrum sampleLi(const Interaction &ref, const Point2f &u, Vector3f *wi,
                       Float *pdf, VisibilityTester *vis) const;
    
    Float falloff(const Vector3f &w) const;
    
    virtual Spectrum power() const;
    
    virtual Float pdfLi(const Interaction &, const Vector3f &) const;
    
private:
    const Point3f _pos;
    const Spectrum _I;
    const Float _cosTotalWidth, _cosFalloffStart;
};

PALADIN_END

#endif /* spot_hpp */
