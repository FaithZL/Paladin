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
	PointLight(const Transform &LightToWorld,
               const MediumInterface &mediumInterface, const Spectrum &I)
	:Light((int)LightFlags::DeltaPosition, LightToWorld, mediumInterface),
	_pLight(LightToWorld.exec(Point3f(0, 0, 0))),
	_I(I) {

	}

	virtual Spectrum sampleLi(const Interaction &ref, const Point2f &u, Vector3f *wi,
                       Float *pdf, VisibilityTester *vis) const;

	Spectrum power() const {
        return 4 * Pi * _I;
    }

    Float pdfLi(const Interaction &, const Vector3f &) const;

    Spectrum sampleLe(const Point2f &u1, const Point2f &u2, Float time,
                       Ray *ray, Normal3f *nLight, Float *pdfPos,
                       Float *pdfDir) const;

    void pdfLe(const Ray &, const Normal3f &, Float *pdfPos,
                Float *pdfDir) const;	

private:
    // 光源位置
    const Point3f _pLight;
    // 辐射强度
    const Spectrum _I;
};

PALADIN_END

#endif /* pointlight_hpp */
