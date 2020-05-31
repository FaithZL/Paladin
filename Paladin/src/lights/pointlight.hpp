//
//  pointlight.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef pointlight_hpp
#define pointlight_hpp

#include "core/light.hpp"
#include "core/shape.hpp"


PALADIN_BEGIN

class PointLight : public Light {
public:
    PointLight(const Transform * LightToWorld,
               const MediumInterface &mediumInterface, const Spectrum &I)
    :Light((int)LightFlags::DeltaPosition, LightToWorld, mediumInterface),
    _pos(LightToWorld->exec(Point3f(0, 0, 0))),
    _I(I) {

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

    virtual Spectrum power() const override {
        return 4 * Pi * _I;
    }

    virtual Float pdf_Li(const Interaction &, const Vector3f &) const override;

    virtual Float pdf_Li(const DirectSamplingRecord &) const override;
private:
    // 光源位置
    const Point3f _pos;
    // 辐射强度
    const Spectrum _I;
};

CObject_ptr createPointLight(const nloJson &param, const Arguments &lst);

PALADIN_END

#endif /* pointlight_hpp */
