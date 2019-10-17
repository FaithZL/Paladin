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
    _pos(LightToWorld.exec(Point3f(0, 0, 0))),
    _I(I) {

    }

    virtual Spectrum sampleLi(const Interaction &ref, const Point2f &u, Vector3f *wi,
                       Float *pdf, VisibilityTester *vis) const;

    virtual Spectrum power() const {
        return 4 * Pi * _I;
    }

    virtual Float pdfLi(const Interaction &, const Vector3f &) const;


private:
    // 光源位置
    const Point3f _pos;
    // 辐射强度
    const Spectrum _I;
};

PALADIN_END

#endif /* pointlight_hpp */
