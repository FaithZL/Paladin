//
//  aggregate.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/8/10.
//

#include "aggregate.hpp"
#include "shape.hpp"
#include "accelerators/bvh.hpp"
#include "math/transform.hpp"

PALADIN_BEGIN

//"data" : {
//    "type" : "bvh",
//    "param" : {
//        "maxPrimsInNode" : 1,
//        "splitMethod" : "SAH"
//    }
//}
shared_ptr<Aggregate> createAccelerator(const nloJson &data, const vector<shared_ptr<const Shape>> &shapes){
    string type = data.value("type", "bvh");
    if (type == "bvh") {
        nloJson param = data.value("param", nloJson::object());
        return createBVH(param, shapes);
    } else if (type == "kdTree") {
        return nullptr;
    }
    DCHECK(false);
    return nullptr;
}

PALADIN_END
