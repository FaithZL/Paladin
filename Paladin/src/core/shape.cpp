//
//  shape.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "shape.hpp"

PALADIN_BEGIN

Shape::Shape(const Transform *objectToWorld, const Transform *worldToObject,
             bool reverseOrientation)
    : _objectToWorld(objectToWorld),
      _worldToObject(worldToObject),
      _reverseOrientation(reverseOrientation),
      _transformSwapsHandedness(objectToWorld->SwapsHandedness()) {

}

Bounds3f Shape::worldBound() const {
	return _objectToWorld->exec(objectBound());
}



PALADIN_END