//
//  header.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/7/1.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef header_h
#define header_h



#include <iostream>
#include <assert.h>
#include <cmath>
#include "stringprint.hpp"
#include "macro.h"
PALADIN_BEGIN
template<typename T>
class Point2;

template<typename T>
class Point3;

template<typename T>
class Vector3;

template<typename T>
class Vector2;

template<typename T>
class Normal3;

template<typename T>
class Bounds3;

template<typename T>
class Bounds2;

template<typename T>
class Direction3;

class Medium;

class Ray;

class Transform;

class Interaction;

class AnimatedTransform;

struct Quaternion;

PALADIN_END

#include "mathutil.h"
#include "vector.h"
#include "point.h"
#include "ray.h"
#include "bounds.h"



#endif /* header_h */
