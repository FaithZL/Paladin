//
//  diffuse.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/18.
//

#include "diffuse.hpp"
#include "core/shape.hpp"

PALADIN_BEGIN

DiffuseAreaLight::DiffuseAreaLight(const Transform &LightToWorld,
                                   const MediumInterface &mediumInterface,
                                   const Spectrum &Lemit, int nSamples,
                                   const std::shared_ptr<Shape> &shape,
                                   bool twoSided)
: AreaLight(LightToWorld, mediumInterface, nSamples),
_Lemit(Lemit),
_shape(shape),
_twoSided(twoSided),
_area(_shape->area()) {
    
}

Spectrum DiffuseAreaLight::power() const {
    return (_twoSided ? _2Pi : Pi) * _Lemit * _area;
}

Spectrum DiffuseAreaLight::sampleLi(const Interaction &ref, const Point2f &u,
                                     Vector3f *wi, Float *pdf,
                                     VisibilityTester *vis) const {
    Interaction pShape = _shape->sampleW(ref, u, pdf);
    pShape.mediumInterface = mediumInterface;
    if (*pdf == 0 || (pShape.pos - ref.pos).lengthSquared() == 0) {
        *pdf = 0;
        return 0.f;
    }
    *wi = normalize(pShape.pos - ref.pos);
    *vis = VisibilityTester(ref, pShape);
    return L(pShape, -*wi);
}

Float DiffuseAreaLight::pdfLi(const Interaction &ref,
                               const Vector3f &wi) const {
    return _shape->pdfW(ref, wi);
}

PALADIN_END
