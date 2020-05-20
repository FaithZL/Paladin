//
//  primitive.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/8/10.
//

#include "primitive.hpp"
#include "shape.hpp"
#include "accelerators/bvh.hpp"
#include "math/transform.hpp"

PALADIN_BEGIN

//GeometricPrimitive
GeometricPrimitive::GeometricPrimitive(const std::shared_ptr<Shape> &shape,
                                       const std::shared_ptr<const Material> &material,
                                       const std::shared_ptr<AreaLight> &areaLight,
                                       const MediumInterface &mediumInterface)
: _shape(shape),
_material(material),
_areaLight(areaLight),
_mediumInterface(mediumInterface) {
    _shape->setPrimitive(this);
}

AABB3f GeometricPrimitive::worldBound() const { 
    return _shape->worldBound(); 
}

AABB3f GeometricPrimitive::objectBound() const {
    return _shape->objectBound();
}

bool GeometricPrimitive::intersectP(const Ray &r) const {
    return _shape->intersectP(r);
}

const Transform & GeometricPrimitive::getWorldToObject() const {
    return * (_shape->worldToObject);
}

const Transform & GeometricPrimitive::getObjectToWorld() const {
    return * (_shape->objectToWorld);
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

bool GeometricPrimitive::fillSurfaceInteraction(const Ray &r, const Vector2f &uv, SurfaceInteraction *isect) const {
    auto ret = _shape->fillSurfaceInteraction(r, uv, isect);
    isect->primitive = this;
    return ret;
}

RTCGeometry GeometricPrimitive::rtcGeometry(Scene *scene) const {
    return _shape->rtcGeometry(scene);
}

EmbreeUtil::EmbreeGeomtry * GeometricPrimitive::getEmbreeGeometry() const {
    return _shape->getEmbreeGeomtry();
}

const AreaLight *GeometricPrimitive::getAreaLight() const {
    return _areaLight.get();
}

const Material *GeometricPrimitive::getMaterial() const {
    return _material.get();
}

shared_ptr<GeometricPrimitive> GeometricPrimitive::create(const std::shared_ptr<Shape> &shape,
                                    const std::shared_ptr<const Material> &material,
                                    const std::shared_ptr<AreaLight> &areaLight,
                                    const MediumInterface &mediumInterface) {
    return make_shared<GeometricPrimitive>(shape, material, areaLight, mediumInterface);
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



//"data" : {
//    "type" : "bvh",
//    "param" : {
//        "maxPrimsInNode" : 1,
//        "splitMethod" : "SAH"
//    }
//}
shared_ptr<Aggregate> createAccelerator(const nloJson &data, const vector<shared_ptr<const Shape>> &shapes){
    string type = data.value("type", "bvh");
    if (type == "bvh") {
        nloJson param = data.value("param", nloJson::object());
        return createBVH(param, shapes);
    } else if (type == "kdTree") {
        return nullptr;
    }
    DCHECK(false);
    return nullptr;
}

PALADIN_END
