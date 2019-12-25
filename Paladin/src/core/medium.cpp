//
//  medium.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/7/16.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "medium.hpp"

PALADIN_BEGIN

Float HenyeyGreenstein::p(const Vector3f &wo, const Vector3f &wi) const {
    return phaseHG(dot(wo, wi), _g);
}

Float HenyeyGreenstein::sample_p(const Vector3f &wo, Vector3f *wi, const Point2f &u) const {
    Float cosTheta = 0;
    if (std::abs(_g) < 1e-3) {
        // 如果各向异性系数很小，则当做各向同性处理
        // 从[0,1)映射到[-1,1)
	    cosTheta = 1 - 2 * u[0];
    } else {
	    // todo 各向异性待推导
	}
    
    Float sinTheta = std::sqrt(std::max((Float)0, 1 - cosTheta * cosTheta));
    
    Float phi = 2 * Pi * u[1];
    Vector3f v1, v2;
    
    coordinateSystem(wo, &v1, &v2);
    *wi = sphericalDirection(sinTheta, cosTheta, phi, v1, v2, -wo);
    return phaseHG(-cosTheta, _g);
}

PALADIN_END
