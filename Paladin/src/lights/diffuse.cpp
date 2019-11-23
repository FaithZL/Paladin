//
//  diffuse.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/18.
//

#include "diffuse.hpp"
#include "core/shape.hpp"

PALADIN_BEGIN

DiffuseAreaLight::DiffuseAreaLight(shared_ptr<const Transform> LightToWorld,
                                   const MediumInterface &mediumInterface,
                                   const Spectrum &L, int nSamples,
                                   const std::shared_ptr<Shape> &shape,
                                   bool twoSided)
: AreaLight(LightToWorld, mediumInterface, nSamples),
_L(L),
_shape(shape),
_twoSided(twoSided),
_area(_shape->area()) {
    
}

Spectrum DiffuseAreaLight::power() const {
    return (_twoSided ? _2Pi : Pi) * _L * _area;
}

Spectrum DiffuseAreaLight::sample_Li(const Interaction &ref, const Point2f &u,
                                     Vector3f *wi, Float *pdf,
                                     VisibilityTester *vis) const {
    Interaction pShape = _shape->sampleDir(ref, u, pdf);
    pShape.mediumInterface = mediumInterface;
    if (*pdf == 0 || (pShape.pos - ref.pos).lengthSquared() == 0) {
        *pdf = 0;
        return 0.f;
    }
    *wi = normalize(pShape.pos - ref.pos);
    *vis = VisibilityTester(ref, pShape);
    return L(pShape, -*wi);
}

Float DiffuseAreaLight::pdf_Li(const Interaction &ref,
                               const Vector3f &wi) const {
    return _shape->pdfDir(ref, wi);
}

DiffuseAreaLight * createDiffuseAreaLight(shared_ptr<const Transform> lightToWorld,
                                          const MediumInterface &mediumInterface, const Spectrum &Le,
                                          int nSamples, const std::shared_ptr<Shape> &shape,
                                          bool twoSided) {
    return new DiffuseAreaLight(lightToWorld, mediumInterface, Le, nSamples, shape, twoSided);
}

//"emission" : {
//    "nSamples" : 1,
//    "Le" : {
//        "colorType" : 1,
//        "color" : [1,1,1],
//    },
//    "twoSided" : false
//}
DiffuseAreaLight * createDiffuse(const nloJson &param,
                                 const std::shared_ptr<Shape> &shape) {
//    shared_ptr<const Transform> o2w = shape->objectToWorld.get();
    
    
}
PALADIN_END
