//
//  geometry.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/28.
//

#include "geometry.hpp"
#include "core/camera.hpp"

PALADIN_BEGIN

//"param" : {
//    "type" : "normal",
//}
CObject_ptr createGeometryIntegrator(const nloJson &param, const Arguments &lst) {
    string type = param.value("type", "normal");
    auto iter = lst.begin();
    Sampler * sampler = dynamic_cast<Sampler *>(*iter);
    ++iter;
    Camera * camera = dynamic_cast<Camera *>(*iter);
    AABB2i pixelBounds = camera->film->getSampleBounds();
    
    return new GeometryIntegrator(shared_ptr<const Camera>(camera), shared_ptr<Sampler>(sampler), pixelBounds, GeometryIntegratorType::Normal);
}

REGISTER("Geometry", createGeometryIntegrator);

PALADIN_END
