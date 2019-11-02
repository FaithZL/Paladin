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
#include "filters/box.h"
#include "cameras/perspective.hpp"
#include "integrators/pathtracer.hpp"
#include "samplers/stratified.hpp"
#include "core/primitive.hpp"

#include "materials/matte.hpp"
#include "lights/distant.hpp"
#include "shapes/sphere.hpp"

#include "textures/constant.h"
#include "accelerators/bvh.hpp"

#include "core/scene.hpp"

#include "tools/parallel.hpp"

using namespace paladin;

void testscene() {
    parallelInit();
    auto pos = Point3f(1,1,10);
    auto target = Point3f(0,0,-1);
    auto up = Vector3f(0,1,0);
    auto lookat = Transform::lookAt(pos,target,up);
    auto aniTrans = AnimatedTransform(&lookat, 0, &lookat, 0);
    
    auto res = Point2i(500,500);
    auto windows = AABB2f(Point2f(0,0), Point2f(1,1));
    std::string fn = "pathtracing.png";
    auto filter = std::unique_ptr<Filter>(new BoxFilter(Vector2f(2,2)));
    auto pFilm = new Film(res, windows, std::move(filter), 35, fn, 1);
    
    auto camera = std::shared_ptr<Camera>(new PerspectiveCamera(aniTrans, AABB2f(Point2f(-1,-1), Point2f(1,1)), 0, 0, 0, 1e6, 30, pFilm, nullptr));
    
    auto sampler = std::shared_ptr<Sampler>(new StratifiedSampler(2, 2, true, 20));
    
    auto pt = new PathTracer(6, camera, sampler, AABB2i(Point2i(0,0), res));
    
    auto lightTrans = Transform(Matrix4x4::identity());
    auto pLight = std::shared_ptr<Light>(new DistantLight(lightTrans,Spectrum(3),Vector3f(-1,-1,0)));
    Float color[3] = {0.1, 0.9, 0.1};
    auto colorKd = std::shared_ptr<ConstantTexture<Spectrum>>(new ConstantTexture<Spectrum>(Spectrum::FromRGB(color, SpectrumType::Illuminant)));
    auto sig = std::shared_ptr<ConstantTexture<Float>>(new ConstantTexture<Float>(0));
    auto matte = std::shared_ptr<Material>(new MatteMaterial(colorKd, sig, nullptr));
    
    auto tralst = Transform::translate(Vector3f(-1,.75, -1.5));
    auto inverse = tralst.getInverse();
    auto sphere = std::shared_ptr<Shape>(new Sphere(&tralst, &inverse, false, 0.75, 0.75, -0.75, 360));
    
    auto gSphere = std::shared_ptr<Primitive>(new GeometricPrimitive(sphere, matte, nullptr, nullptr));
    
    std::vector<std::shared_ptr<Primitive>> lst;
    lst.push_back(gSphere);
    
    auto bvh = std::shared_ptr<Primitive>(new BVHAccel(lst));
    std::vector<std::shared_ptr<Light>> lights;
    lights.push_back(pLight);
    auto scene = paladin::Scene(bvh, lights);
    
    pt->render(scene);
}

#endif /* testrender_h */
