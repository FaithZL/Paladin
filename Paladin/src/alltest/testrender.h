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
#include "core/primitive.hpp"

#include "materials/matte.hpp"
#include "lights/distant.hpp"
#include "shapes/sphere.hpp"

#include "lights/diffuse.hpp"

#include "textures/constant.h"
#include "accelerators/bvh.hpp"

#include "core/scene.hpp"

#include "tools/parallel.hpp"
#include "lights/envmap.hpp"


using namespace paladin;
using namespace std;

void testscene() {
    parallelInit();
    auto pos = Point3f(0,0,-5);
    auto target = Point3f(0,0,0);
    auto up = Vector3f(0,1,0);
    auto c2w = Transform::lookAt(pos,target,up).getInverse_ptr();
    auto aniTrans = AnimatedTransform(shared_ptr<const Transform>(c2w), 0, shared_ptr<const Transform>(c2w), 0);
    
    auto res = Point2i(400,400);
    auto windows = AABB2f(Point2f(0,0), Point2f(1,1));
    std::string fn = "pathtracing.png";
    auto filter = std::unique_ptr<Filter>(new BoxFilter(Vector2f(2,2)));
    auto pFilm = new Film(res, windows, std::move(filter), 45, fn, 1);
    
    auto camera = std::shared_ptr<Camera>(new PerspectiveCamera(aniTrans, AABB2f(Point2f(-1,-1), Point2f(1,1)), 0, 0, 0, 1e6, 45, std::shared_ptr<Film>(pFilm), nullptr));
    
    auto sampler = std::shared_ptr<Sampler>(new StratifiedSampler(3, 3, true, 20));
    
    auto pt = new PathTracer(6, camera, sampler, AABB2i(Point2i(0,0), res));
    
    auto lightTrans = Transform(Matrix4x4::identity());
    Float srgb[3] = {1, 0.2, 0.5};
    auto pLight = std::shared_ptr<Light>(new DistantLight(lightTrans,Spectrum::FromRGB(srgb),Vector3f(1,1,0)));
    Float color[3] = {0.1, 0.9, 0.1};
    auto colorKd = std::shared_ptr<ConstantTexture<Spectrum>>(new ConstantTexture<Spectrum>(Spectrum::FromRGB(color, SpectrumType::Illuminant)));
    auto sig = std::shared_ptr<ConstantTexture<Float>>(new ConstantTexture<Float>(0));
    auto matte = std::shared_ptr<Material>(new MatteMaterial(colorKd, sig, nullptr));
    
    auto tralst = Transform::translate_ptr(Vector3f(0,0, 0));
    auto scale = Transform::scale(1.2, 1, 1);
    
    auto envpath = "res/derelict_overpass_1k.hdr";
    auto el = Spectrum(1.0f);
    auto env2w = Transform::rotateX(-90);
    auto env = std::shared_ptr<Light>(new EnvironmentMap(env2w, el,10, envpath));
    
    auto inverse = tralst->getInverse_ptr();
    auto sphere = std::shared_ptr<Shape>(new Sphere(shared_ptr<const Transform>(tralst), shared_ptr<const Transform>(inverse), false, 0.75, 0.75, -0.75, 360));
    
    auto gSphere = std::shared_ptr<Primitive>(new GeometricPrimitive(sphere, matte, nullptr, nullptr));
    auto mi = MediumInterface(nullptr);
    
    auto t2 = Transform::translate_ptr(Vector3f(-2,0,0));
    auto int2 = t2->getInverse_ptr();
    auto lsphere = std::shared_ptr<Shape>(new Sphere(shared_ptr<const Transform>(t2), shared_ptr<const Transform>(int2), false, 0.75, 0.75, -0.75, 360));
    auto areaL = std::shared_ptr<AreaLight>(new DiffuseAreaLight(*tralst, mi, Spectrum(1), 10, lsphere));
    auto l2 = std::shared_ptr<Primitive>(new GeometricPrimitive
                                         (lsphere, nullptr, areaL, nullptr));
    std::vector<std::shared_ptr<Primitive>> lst;
    lst.push_back(l2);
    lst.push_back(gSphere);
    
    auto bvh = std::shared_ptr<Primitive>(new BVHAccel(lst));
    std::vector<std::shared_ptr<Light>> lights;
    lights.push_back(areaL);
    lights.push_back(pLight);
    
//    lights.push_back(env);
    
    auto scene = paladin::Scene(bvh, lights);
    
    pt->render(scene);
    parallelCleanup();
}

#endif /* testrender_h */
