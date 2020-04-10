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

void Scene::accelInitEmbree(const vector<shared_ptr<Primitive> > &primitives) {
    _rtcScene = rtcNewScene(EmbreeUtil::getDevice());
    vector<RTCGeometry> gList;
    for (auto iter = primitives.cbegin(); iter != primitives.cend(); ++iter) {
        auto prim = *iter;
        RTCGeometry gid = prim->embreeGeometry(this);
        if (gid != nullptr) {
            gList.push_back(gid);
            rtcAttachGeometry(_rtcScene, gid);
        }
    }
    rtcCommitScene(_rtcScene);
}

PALADIN_END
