//
//  scene.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "scene.hpp"
#include "tools/embree_util.hpp"
#include "shapes/mesh.hpp"

PALADIN_BEGIN

void Scene::initEnvmap() {
    for (const auto &light : lights) {
        light->preprocess(*this);
        if (light->flags & (int)LightFlags::Infinite) {
            infiniteLights.push_back(light);
        }
    }
}

bool Scene::rayIntersectTr(Ray ray, Sampler &sampler, SurfaceInteraction *isect,
                        Spectrum *Tr) const {
    *Tr = Spectrum(1.f);
    while (true) {
        bool hitSurface = rayIntersect(ray, isect);
        if (ray.medium) {
            *Tr = ray.medium->Tr(ray, sampler);
        }
        
        if (!hitSurface) {
            return false;
        }
        
        if (isect->shape->getMaterial() != nullptr) {
             return true;
        }
        ray = isect->spawnRay(ray.dir);
    }
}

void Scene::initAccel(const nloJson &data, const vector<shared_ptr<const Shape> > &shapes) {
    string type = data.value("type", "embree");
    _shapes = shapes;
    if (type == "embree") {
        InitAccelEmbree(shapes);
    } else {
        InitAccelNative(data, shapes);
    }
}

Spectrum Scene::sampleLightDirect(DirectSamplingRecord *rcd, const Point2f _u,
                                  const Distribution1D *lightDistrib,
                                  Float *pmf) const {
    Point2f u(_u);
    Float index = lightDistrib->sampleDiscrete(u.x, pmf, &u.x);
    const Light * light = lights.at(index).get();
    rcd->object = light;
//    Vector3f wi;
//    Float pdf;
//    VisibilityTester vis;
//    auto r = light->sample_Li(Interaction(rcd->ref()), u, &wi, &pdf, &vis);
    auto r2 = light->sample_Li(rcd, u, *this);
    return r2;
}

bool Scene::rayIntersectEmbree(const Ray &ray, SurfaceInteraction *isect) const {
    RTCIntersectContext context;
    rtcInitIntersectContext(&context);
    RTCRayHit rh = EmbreeUtil::toRTCRayHit(ray);
    rtcIntersect1(_rtcScene, &context, &rh);
    if (rh.hit.geomID == RTC_INVALID_GEOMETRY_ID) {
        return false;
    }
    uint32_t gid = rh.hit.geomID;
    uint32_t pid = rh.hit.primID;
    Vector2f uv(rh.hit.u, rh.hit.v);
    
    const Shape * shape = _shapes.at(gid).get();
    switch (shape->getType()) {
        case EMesh: {
            auto mesh = static_cast<const Mesh *>(shape);
            auto tri = mesh->getTriangle(pid);
            tri->fillSurfaceInteraction(ray, uv, isect);
            break;
        }
        default:
            break;
    }
    return true;
}

void Scene::InitAccelNative(const nloJson &data, const vector<shared_ptr<const Shape>> &shapes) {
    
    _aggregate = createAccelerator(data, shapes);
    _worldBound = _aggregate->worldBound();
    initEnvmap();
}

void Scene::InitAccelEmbree(const vector<shared_ptr<const Shape>>&shapes) {
    EmbreeUtil::initDevice();
    _rtcScene = rtcNewScene(EmbreeUtil::getDevice());
    int idx = 0;
    for (size_t i = 0; i < shapes.size(); ++i) {
        auto prim = shapes[i];
        RTCGeometry gid = prim->rtcGeometry(this);
        _worldBound = unionSet(_worldBound, prim->worldBound());
        if (gid != nullptr) {
            rtcAttachGeometry(_rtcScene, gid);
        }
    }
    initEnvmap();
    rtcCommitScene(_rtcScene);
}

PALADIN_END
