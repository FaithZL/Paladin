//
//  shape.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "shape.hpp"

PALADIN_BEGIN

Shape::~Shape() {
    
}

Shape::Shape(const Transform *objectToWorld, const Transform *worldToObject,
             bool reverseOrientation)
    : _objectToWorld(objectToWorld),
      _worldToObject(worldToObject),
      _reverseOrientation(reverseOrientation),
      _transformSwapsHandedness(objectToWorld->swapsHandedness()) {

}

Bounds3f Shape::worldBound() const {
    return _objectToWorld->exec(objectBound());
}
/*
由立体角定义 dw/dA = cosθ/r^2
联合表达式 py(y) * dy/dx = px(x)
可得 p(w) = r^2 / (A * cosθ) * p(A)
 */
//Interaction Shape::sample(const Interaction &ref, const Point2f &u, Float *pdf) const {
//    Interaction intr = sample(u, pdf);
//    Vector3f wi = intr.pos - ref.pos;
//    if (wi.lengthSquared() == 0) {
//        *pdf = 0.f;
//    } else {
//        wi = normalize(wi);
//        *pdf *= distanceSquared(ref.pos, intr.pos) / absDot(intr.normal, -wi);
//        if (std::isinf(*pdf)) {
//            *pdf = 0.f;
//        }
//    }
//    return intr;
//}
//
//
Float Shape::pdf(const Interaction &ref, const Vector3f &wi) const {
   Ray ray = ref.spawnRay(wi);
//    Float tHit;
//    SurfaceInteraction isectLight;
    return 0;
}

PALADIN_END
