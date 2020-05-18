//
//  light.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "light.hpp"
#include "core/scene.hpp"
#include "math/sampling.hpp"

PALADIN_BEGIN


bool VisibilityTester::unoccluded(const Scene &scene) const {
    return !scene.rayOccluded(_p0.spawnRayTo(_p1));
}

Spectrum VisibilityTester::Tr(const Scene &scene, Sampler &sampler) const {
    Spectrum ret(1.f);
    Ray ray = _p0.spawnRayTo(_p1);
    while (true) {
        SurfaceInteraction isect;
        bool hitSurface = scene.rayIntersect(ray, &isect);
        // 如果有交点，且交点处材质不为空，则返回0，透明材质不在此处理
        if (hitSurface && isect.primitive->getMaterial() != nullptr) {
            return Spectrum(0.f);
        }
        
        if (ray.medium) {
            ret *= ray.medium->Tr(ray, sampler);
        }
        
        if (!hitSurface) {
            break;
        }
        // 交点处的材质为空时会运行到此，重新生成ray
        ray = isect.spawnRayTo(_p1);
    }
    return ret;
}

//"data" : {
//    "type" : "pointLight",
//    "param" : {
//        "worldToLocal" : {
//            "type" : "translate",
//            "param" : [1,0,1]
//        },
//        "I" : {
//            "colorType" : 1,
//            "color" : [0.1, 0.9, 0.5]
//        },
//    }
//}
Light * createLight(const nloJson &data) {
    bool enable = data.value("enable", true);
    if (!enable) {
        return nullptr;
    }
    string type = data.value("type", "pointLight");
    auto creator = GET_CREATOR(type);
    nloJson param = data.value("param", nloJson::object());
    auto ret = dynamic_cast<Light *>(creator(param, {}));
    DCHECK(ret != nullptr);
    return ret;
}


PALADIN_END
