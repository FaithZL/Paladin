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
    return !scene.intersectP(_p0.spawnRayTo(_p1));
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
    string type = data.value("type", "pointLight");
    auto creator = GET_CREATOR(type);
    nloJson param = data.value("param", nloJson::object());
    auto ret = dynamic_cast<Light *>(creator(param, {}));
    DCHECK(ret != nullptr);
    return ret;
}


PALADIN_END
