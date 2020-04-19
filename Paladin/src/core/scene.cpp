//
//  scene.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "scene.hpp"
#include "tools/embree_util.hpp"

PALADIN_BEGIN

void Scene::initEnvmap() {
    for (const auto &light : lights) {
        light->preprocess(*this);
        if (light->flags & (int)LightFlags::Infinite) {
            infiniteLights.push_back(light);
        }
    }
}

bool Scene::intersectTr(Ray ray, Sampler &sampler, SurfaceInteraction *isect,
                        Spectrum *Tr) const {
    *Tr = Spectrum(1.f);
    while (true) {
        bool hitSurface = intersect(ray, isect);
        if (ray.medium) {
            *Tr = ray.medium->Tr(ray, sampler);
        }
        
        if (!hitSurface) {
            return false;
        }
        
        if (isect->primitive->getMaterial() != nullptr) {
             return true;
        }
        ray = isect->spawnRay(ray.dir);
    }
}

bool Scene::embreeIntersect(const Ray &ray, SurfaceInteraction *isect) const {
    RTCIntersectContext context;
    rtcInitIntersectContext(&context);
    RTCRayHit rh = EmbreeUtil::toRTCRayHit(ray);
    rtcIntersect1(_rtcScene, &context, &rh);
    if (rh.hit.geomID == RTC_INVALID_GEOMETRY_ID) {
        return false;
    }
    uint32_t gid = rh.hit.geomID;
    uint32_t pid = rh.hit.primID;
    Primitive * prim = getPrimitive(gid, pid);
    
    Vector2f uv(rh.hit.u, rh.hit.v);
    
    bool ret = prim->fillSurfaceInteraction(ray, uv, isect);
    return true;
}


bool Scene::intersect(const Ray &ray, SurfaceInteraction *isect) const {
        CHECK_NE(ray.dir, Vector3f(0,0,0));
    if (_rtcScene) {
         return embreeIntersect(ray, isect);
    }
    return _aggregate->intersect(ray, isect);
}


bool Scene::intersectP(const Ray &ray) const {
    CHECK_NE(ray.dir, Vector3f(0,0,0));
    if (_rtcScene) {
        return embreeIntersectP(ray);
    }
    return _aggregate->intersectP(ray);
}

bool Scene::embreeIntersectP(const Ray &ray) const {
    using namespace EmbreeUtil;
    RTCIntersectContext context;
    rtcInitIntersectContext(&context);
    RTCRay r = EmbreeUtil::convert(ray);
    rtcOccluded1(_rtcScene, &context, &r);
    return r.tfar < 0;
}

EmbreeUtil::EmbreeGeomtry * Scene::getEmbreeGeomtry(int geomID) const {
    return _embreeGeometries[geomID];
}

Primitive * Scene::getPrimitive(int geomID, int primID) const {
    EmbreeUtil::EmbreeGeomtry * ret = _embreeGeometries[geomID];
    auto shape = ret->getShape(primID);
    auto primitive = shape->getPrimitive();
    return primitive;
}

//"data" : {
//    "type" : "bvh",
//    "param" : {
//        "maxPrimsInNode" : 1,
//        "splitMethod" : "SAH"
//    }
//}
void Scene::initAccel(const nloJson &data, const vector<shared_ptr<Primitive> > &primitives) {
    string type = data.value("type", "embree");
    if (type == "embree") {
        accelInitEmbree(primitives);
    } else {
        accelInitNative(data, primitives);
    }
}

void Scene::accelInitNative(const nloJson &data,
                            const vector<shared_ptr<Primitive> >&primitives) {
    _aggregate = createAccelerator(data, primitives);
    _worldBound = _aggregate->worldBound();
    initEnvmap();
}

void Scene::accelInitEmbree(const vector<shared_ptr<Primitive> > &primitives) {
    EmbreeUtil::initDevice();
    _rtcScene = rtcNewScene(EmbreeUtil::getDevice());
    int idx = 0;
    for (auto iter = primitives.cbegin(); iter != primitives.cend(); ++iter) {
        auto prim = *iter;
        RTCGeometry gid = prim->rtcGeometry(this);
        _worldBound = unionSet(_worldBound, prim->worldBound());
        if (gid != nullptr) {
            _embreeGeometries.push_back(prim->getEmbreeGeometry());
            rtcAttachGeometry(_rtcScene, gid);
        }
    }
    rtcCommitScene(_rtcScene);
    initEnvmap();
}

PALADIN_END
