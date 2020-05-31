//
//  testrender.h
//  Paladin
//
//  Created by SATAN_Z on 2019/10/27.
//

#ifndef testrender_h
#define testrender_h

#include "math/transform.hpp"
#include "math/animatedtransform.hpp"
#include "core/film.hpp"
#include "filters/box.hpp"
#include "cameras/perspective.hpp"
#include "integrators/pathtracer.hpp"
#include "samplers/stratified.hpp"
#include "samplers/random.hpp"
#include "core/aggregate.hpp"

#include "materials/matte.hpp"
#include "lights/distant.hpp"
#include "shapes/sphere.hpp"

#include "lights/diffuse.hpp"

#include "textures/constant.hpp"
#include "accelerators/bvh.hpp"

#include "core/scene.hpp"

#include "tools/parallel.hpp"
#include "lights/envmap.hpp"
#include "ext/nlohmann/json.hpp"


using namespace paladin;
using namespace std;
using namespace nlohmann;

void testscene() {
    
    BoxFilter * flt = new BoxFilter(Vector2f(1.0, 1.0));
    
    json filmData = R"(
        {
            "resolution" : [500, 500],
            "cropWindow" : [0,0,1,1],
            "fileName" : "paladin.png",
            "diagonal" : 1,
            "scale" : 1
        }
    )"_json;
    
    auto film = dynamic_cast<Film *>(createFilm(filmData, {flt}));
    
    json patch = R"({
        "shutterOpen" : 0,
        "shutterClose" : 1,
        "lensRadius" : 0,
        "focalDistance" : 100,
        "fov" : 45,
        "aspect" : 1,
        "lookAt" : [
            [0,0,-5],
            [0,0,0],
            [0,1,0]
        ],
        "lookAtEnd" : [
            [0,0,-5],
            [0,0,0],
            [0,1,0]
        ]
    })"_json;
    
    
    auto camera = dynamic_cast<PerspectiveCamera *>(createPerspectiveCamera(patch, {film}));
    
    json samplerData = R"( {
         "spp" : 8
    })"_json;
    
    Sampler * sampler = dynamic_cast<Sampler*>(createRandomSampler(samplerData));
    
    Interaction intr;
    Spectrum sum;
    int n = 100;
    Vector3f wi;
    Float pdf;
    VisibilityTester vis;
//    intr.pos.z += 10;
    for(int i = 0; i < n ; ++i) {
        auto we = camera->sample_Wi(intr, sampler->get2D(), &wi, &pdf, nullptr, &vis);
        sum += we / pdf;
    }
    COUT << sum;
    
}

#endif /* testrender_h */
