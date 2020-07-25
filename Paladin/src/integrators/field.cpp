//
//  geometry.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/28.
//

#include "field.hpp"
#include "core/camera.hpp"

PALADIN_BEGIN

//"param" : {
//    "type" : "normal",
//}
CObject_ptr createFieldIntegrator(const nloJson &param, const Arguments &lst) {
    string type = param.value("type", "sNormal");
    auto iter = lst.begin();
    Sampler * sampler = dynamic_cast<Sampler *>(*iter);
    ++iter;
    Camera * camera = dynamic_cast<Camera *>(*iter);
    AABB2i pixelBounds = camera->film->getSampleBounds();
    
    return new FieldIntegrator(shared_ptr<const Camera>(camera), shared_ptr<Sampler>(sampler), pixelBounds, type);
}

REGISTER("field", createFieldIntegrator);

PALADIN_END
