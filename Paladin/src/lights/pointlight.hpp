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
	_pLight(LightToWorld(Point3f(0, 0, 0))),
	_I(I) {

	}

private:
    const Point3f _pLight;
    const Spectrum _I;
};

PALADIN_END

#endif /* pointlight_hpp */
