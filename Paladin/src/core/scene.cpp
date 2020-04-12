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
    using namespace EmbreeUtil;
    RTCRay rtcRay = convert(ray);
    RTCIntersectContext context;
    rtcInitIntersectContext(&context);
    RTCRayHit rh;
    rh.ray = rtcRay;
    rh.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    rh.hit.primID = RTC_INVALID_GEOMETRY_ID;
    rtcIntersect1(_rtcScene, &context, &rh);
    int gid = rh.hit.geomID;
    int pid = rh.hit.primID;
    
    auto p = getEmbreeGeomtry(gid, pid);
}

EmbreeUtil::EmbreeGeomtry * Scene::getEmbreeGeomtry(int geomID, int primID) const {
    EmbreeUtil::EmbreeGeomtry * ret = _embreeGeometries[geomID];
    return ret->getShape(primID);
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
//    if (type == "embree") {
        accelInitEmbree(primitives);
//    } else {
        accelInitNative(data, primitives);
//    }
}

void Scene::accelInitEmbree(const vector<shared_ptr<Primitive> > &primitives) {
    EmbreeUtil::initDevice();
    _rtcScene = rtcNewScene(EmbreeUtil::getDevice());
    int idx = 0;
    for (auto iter = primitives.cbegin(); iter != primitives.cend(); ++iter) {
        auto prim = *iter;
        RTCGeometry gid = prim->rtcGeometry(this);
        if (gid != nullptr) {
            _embreeGeometries.push_back(prim->getEmbreeGeometry());
            rtcAttachGeometry(_rtcScene, gid);
        }
    }
    rtcCommitScene(_rtcScene);
}

PALADIN_END
