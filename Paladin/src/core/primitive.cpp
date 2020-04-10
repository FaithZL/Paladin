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
    return * (_shape->worldToObject.get());
}

const Transform & GeometricPrimitive::getObjectToWorld() const {
    return * (_shape->objectToWorld.get());
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

RTCGeometry GeometricPrimitive::embreeGeometry(Scene *scene) const {
    return _shape->embreeGeometry(scene);
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

//TransformedPrimitive
shared_ptr<TransformedPrimitive> TransformedPrimitive::create(const std::shared_ptr<Primitive> &primitive,
                                                              const shared_ptr<const Transform> &o2w,
                                                              const std::shared_ptr<const Material> &mat,
                                                              const MediumInterface &mediumInterface) {
    return make_shared<TransformedPrimitive>(primitive, o2w, mat);
}

TransformedPrimitive::TransformedPrimitive(const std::shared_ptr<Primitive> &primitive,
                                          const shared_ptr<const Transform> &o2w,
                                           const std::shared_ptr<const Material> &mat,
                                           const MediumInterface &mediumInterface):
_primitive(primitive),
_material(mat),
_mediumInterface(mediumInterface) {
    shared_ptr<GeometricPrimitive> prim = dynamic_pointer_cast<GeometricPrimitive>(primitive);
    const Transform & trf = prim->getWorldToObject();
    auto w2o = (*o2w) * (trf);
    _objectToWorld = w2o;
}

bool TransformedPrimitive::intersect(const Ray &r,
                                     SurfaceInteraction *isect) const {
    // 插值获取primitive到world的变换

    // 将局部坐标转换为世界坐标
    Ray ray = _objectToWorld.getInverse().exec(r);
    
    if (!_primitive->intersect(ray, isect)) {
        return false;
    }
    // 更新tMax
    r.tMax = ray.tMax;

    if (!_objectToWorld.isIdentity()) {
        *isect = _objectToWorld.exec(*isect);
    }
    
    if (_mediumInterface.isMediumTransition()){
        isect->mediumInterface = _mediumInterface;
    } else {
        isect->mediumInterface = MediumInterface(r.medium);
    }
    isect->primitive = this;
    CHECK_GE(dot(isect->normal, isect->shading.normal), 0);
    return true;
}

bool TransformedPrimitive::intersectP(const Ray &r) const {
    Transform InterpolatedPrimToWorld = _objectToWorld;
    Transform InterpolatedWorldToPrim = InterpolatedPrimToWorld.getInverse();
    return _primitive->intersectP(InterpolatedWorldToPrim.exec(r));
}

//"data" : {
//    "type" : "bvh",
//    "param" : {
//        "maxPrimsInNode" : 1,
//        "splitMethod" : "SAH"
//    }
//}
shared_ptr<Aggregate> createAccelerator(const nloJson &data, const vector<shared_ptr<Primitive>> &prims){
    string type = data.value("type", "bvh");
    if (type == "bvh") {
        nloJson param = data.value("param", nloJson::object());
        return createBVH(param, prims);
    } else if (type == "kdTree") {
        return nullptr;
    }
    DCHECK(false);
    return nullptr;
}

PALADIN_END
