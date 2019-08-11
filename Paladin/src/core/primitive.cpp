//
//  primitive.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/8/10.
//

#include "primitive.hpp"
#include "shape.hpp"

PALADIN_BEGIN


GeometricPrimitive::GeometricPrimitive(const std::shared_ptr<Shape> &shape,
                                       const std::shared_ptr<Material> &material,
                                       const std::shared_ptr<AreaLight> &areaLight,
                                       const MediumInterface &mediumInterface)
: _shape(shape),
_material(material),
_areaLight(areaLight),
_mediumInterface(mediumInterface) {
    
}

AABB3f GeometricPrimitive::worldBound() const { 
    return _shape->worldBound(); 
}

bool GeometricPrimitive::intersectP(const Ray &r) const {
    return _shape->intersectP(r);
}

bool GeometricPrimitive::intersect(const Ray &r,
                                   SurfaceInteraction *isect) const {
    Float tHit;
    if (!_shape->intersect(r, &tHit, isect)) {
        return false;
    }
    r.tMax = tHit;
    isect->primitive = this;
    CHECK_GE(dot(isect->normal, isect->shading.normal), 0.);

    if (_mediumInterface.isMediumTransition()){
        isect->mediumInterface = _mediumInterface;
    } else {
        isect->mediumInterface = MediumInterface(r.medium);
    }
    return true;
}

const AreaLight *GeometricPrimitive::getAreaLight() const {
    return _areaLight.get();
}

const Material *GeometricPrimitive::getMaterial() const {
    return _material.get();
}

void GeometricPrimitive::computeScatteringFunctions(
                                                    SurfaceInteraction *isect, MemoryArena &arena, TransportMode mode,
                                                    bool allowMultipleLobes) const {
    if (_material) {
        _material->computeScatteringFunctions(isect, arena, mode,
                                             allowMultipleLobes);
    }
    CHECK_GE(dot(isect->normal, isect->shading.normal), 0.);
}


PALADIN_END
