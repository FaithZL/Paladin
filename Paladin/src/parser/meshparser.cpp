//
//  meshparser.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/3/1.
//

#include "meshparser.hpp"
#include "materials/matte.hpp"
#include "lights/diffuse.hpp"
#include "core/primitive.hpp"
#include "core/paladin.hpp"
#include "modelcache.hpp"

PALADIN_BEGIN


// data : {
//     "type" : "triMesh",
//     "subType" : "mesh",
//     "param" : {
//         "normals" : [
//             1,0,0,
//             2,0,0
//         ],
//         "verts" : [
//             2,1,1,
//             3,2,1
//         ],
//         "UVs" : [
//             0.9,0.3,
//             0.5,0.6
//         ],
//         "indexes" : [
//             [1,2,3],
//             [3,5,6]
//         ],
//         "material" : [
//             {
//                 "type" : "unity",
//                 "param" : {
//                     "albedo" : [0.725, 0.71, 0.68],
//                     "roughness" : 0.2,
//                     "metallic" : 0.8
//                 }
//             },
//             {
//                 "type" : "unity",
//                 "param" : {
//                     "albedo" : [0.725, 0.71, 0.68],
//                     "roughness" : 0.2,
//                     "metallic" : 0.8
//                 }
//             }
//         ],
//         "transform" : {
//             "type" : "matrix",
//             "param" : [
//                 1,0,0,0,
//                 0,1,0,0,
//                 0,0,1,0,
//                 0,0,0,1
//             ]
//         },
//         "emission" : {
//             "nSamples" : 1,
//             "Le" : {
//                 "colorType" : 1,
//                 "color" : [1,1,1]
//             },
//             "twoSided" : false
//         }
//     },
//     "transform" : {
//         "type" : "matrix",
//         "param" : [
//             1,0,0,0,
//             0,1,0,0,
//             0,0,1,0,
//             0,0,0,1
//         ]
//     },
// },
vector<shared_ptr<Primitive>> MeshParser::getPrimitiveLst(const nloJson &data,
                                                          vector<shared_ptr<Light>> &lights) {
    nloJson param = data.value("param", nloJson());
    
    nloJson transformData = data.value("transform", nloJson());
    auto transform(createTransform(transformData));
    
    if (param.is_string()) {
        string fn = param;
        fn = Paladin::getInstance()->getBasePath() + fn;
        return ModelCache::getInstance()->getPrimitives(fn, transform, lights);
    }
    
    return ModelCache::createPrimitive(param, transform, lights);
}

PALADIN_END
