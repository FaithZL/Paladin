//
//  primitive.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/8/10.
//

#include "primitive.hpp"
#include "shape.hpp"
#include "accelerators/bvh.hpp"

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
TransformedPrimitive::TransformedPrimitive(std::shared_ptr<Primitive> &primitive,
                                           const AnimatedTransform &PrimitiveToWorld,
                                           const std::shared_ptr<const Material> &mat):
_primitive(primitive),
_primitiveToWorld(PrimitiveToWorld),
_material(mat) {
    
}

bool TransformedPrimitive::intersect(const Ray &r,
                                     SurfaceInteraction *isect) const {
    // 插值获取primitive到world的变换
    Transform InterpolatedPrimToWorld = _primitiveToWorld.interpolate(r.time);
    // 将局部坐标转换为世界坐标
    Ray ray = InterpolatedPrimToWorld.getInverse().exec(r);
    
    if (!_primitive->intersect(ray, isect)) {
        return false;
    }
    // 更新tMax
    r.tMax = ray.tMax;

    if (!InterpolatedPrimToWorld.isIdentity()) {
        *isect = InterpolatedPrimToWorld.exec(*isect);
    }
    CHECK_GE(dot(isect->normal, isect->shading.normal), 0);
    return true;
}

bool TransformedPrimitive::intersectP(const Ray &r) const {
    Transform InterpolatedPrimToWorld = _primitiveToWorld.interpolate(r.time);
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
