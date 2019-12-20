//
//  volpath.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/21.
//

#include "volpath.hpp"
#include "core/camera.hpp"

PALADIN_BEGIN

VolumePathTracer::VolumePathTracer(int maxDepth, std::shared_ptr<const Camera> camera,
                       std::shared_ptr<Sampler> sampler,
                       const AABB2i &pixelBounds, Float rrThreshold /* = 1*/,
                       const std::string &lightSampleStrategy /*= "power"*/)
: MonteCarloIntegrator(camera, sampler, pixelBounds),
_maxDepth(maxDepth),
_rrThreshold(rrThreshold),
_lightSampleStrategy(lightSampleStrategy) {
    
}

void VolumePathTracer::preprocess(const Scene &scene, Sampler &sampler) {
    _lightDistribution = createLightSampleDistribution(_lightSampleStrategy, scene);
}

Spectrum VolumePathTracer::Li(const RayDifferential &r, const Scene &scene,
						Sampler &sampler, MemoryArena &arena, int depth) const {
	Spectrum L(0.f);
	Spectrum throughput (1.f);
	RayDifferential ray(r);
	bool specularBounce = false;
	int bounce;

	Float etaScale = 1.f;

	for (bounce = 0; ; ++bounce) {
		SurfaceInteration isect;
		bool foundIntersection = scene.intersectP(ray, &isect);
		// 如果当前ray是直接从相机发射，
		// 判断光线是否与场景几何图元相交
		if (specularBounce || bounce == 0) {
			// 因为在这种情况下，ray的方向是确定的
			
		}
	}
}

//"param" : {
//    "maxBounce" : 5,
//    "rrThreshold" : 1,
//    "lightSampleStrategy" : "power"
//}
// lst = {sampler, camera}
CObject_ptr createVolumePathTracer(const nloJson &param, const Arguments &lst) {
    int maxBounce = param.value("maxBounce", 5);
    Float rrThreshold = param.value("rrThreshold", 1.f);
    string lightSampleStrategy = param.value("lightSampleStrategy", "power");
    auto iter = lst.begin();
    Sampler * sampler = dynamic_cast<Sampler *>(*iter);
    ++iter;
    Camera * camera = dynamic_cast<Camera *>(*iter);
    AABB2i pixelBounds = camera->film->getSampleBounds();
    VolumePathTracer * ret = new VolumePathTracer(maxBounce,
                                      shared_ptr<const Camera>(camera),
                                      shared_ptr<Sampler>(sampler),
                                      pixelBounds,
                                      rrThreshold,
                                      lightSampleStrategy);
    
    return ret;
}

REGISTER("volpt", createVolumePathTracer);


PALADIN_END