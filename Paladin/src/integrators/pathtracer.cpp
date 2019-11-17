//
//  pathtracer.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "pathtracer.hpp"
#include "core/camera.hpp"

PALADIN_BEGIN

PathTracer::PathTracer(int maxDepth, std::shared_ptr<const Camera> camera,
                       std::shared_ptr<Sampler> sampler,
                       const AABB2i &pixelBounds, Float rrThreshold /* = 1*/,
                       const std::string &lightSampleStrategy /*= "power"*/)
: MonteCarloIntegrator(camera, sampler, pixelBounds),
_maxDepth(maxDepth),
_rrThreshold(rrThreshold),
_lightSampleStrategy(lightSampleStrategy) {
    
}

PathTracer::PathTracer(int maxDepth,
                       const AABB2i &pixelBounds,
                       Float rrThreshold /*= 1*/,
                       const std::string &lightSampleStrategy/*( = "power"*/)
: MonteCarloIntegrator(pixelBounds),
_maxDepth(maxDepth),
_rrThreshold(rrThreshold),
_lightSampleStrategy(lightSampleStrategy){
    
}

void PathTracer::preprocess(const Scene &scene, Sampler &sampler) {
    _lightDistribution = createLightSampleDistribution(_lightSampleStrategy, scene);
}

Spectrum PathTracer::Li(const RayDifferential &r, const Scene &scene,
						Sampler &sampler, MemoryArena &arena, int depth) const {
	Spectrum L(0.0f);
	Spectrum throughput(1.0f);
	RayDifferential ray(r);
	bool specularBounce = false;
	int bounces;

	Float etaScale = 1;

	for (bounces = 0;; ++bounces) {
		SurfaceInteraction isect;
		bool foundIntersection = scene.intersect(ray, &isect);
		// 如果当前ray是直接从相机发射，
		// 判断光线是否与场景几何土元相交
		if (bounces == 0 || specularBounce) {
			// 如果与几何土元有交点，则判断是否为光源，估计Le
			if (foundIntersection) {
				L += throughput * isect.Le(-ray.dir);
			} else {
				// 如果没有交点，则采样环境光
				for (const auto &light : scene.infiniteLights) {
					L += throughput * light->Le(ray);
				}
			}
		}

		if (!foundIntersection || bounces >= _maxDepth) {
			break;
		}

		isect.computeScatteringFunctions(ray, arena, true);
		// 如果没有bsdf，则不计算反射次数
		// 有些几何图元是仅仅是为了限定参与介质的范围
		// 所以没有bsdf
		if (!isect.bsdf) {
			ray = isect.spawnRay(ray.dir);
			--bounces;
			continue;
		}

		const Distribution1D * distrib = _lightDistribution->lookup(isect.pos);
		// 找到非高光反射comp，如果有，则估计直接光照贡献
		if (isect.bsdf->numComponents(BxDFType(BSDF_ALL & ~BSDF_SPECULAR)) > 0) {
			Spectrum Ld = throughput * sampleOneLight(isect, scene, arena, sampler, false, distrib);

			L += Ld;
		}

		// 开始采样BSDF，生成wi方向，追踪更长的路径
		Vector3f wo = -ray.dir;
		Vector3f wi;
		Float pdf;
		BxDFType flags;
		Spectrum f = isect.bsdf->sample_f(wo, &wi, sampler.get2D(), &pdf, BSDF_ALL, &flags);

		if (f.IsBlack() || pdf == 0.0f) {
			break;
		}
		/**
		 * 复用之前的路径对吞吐量进行累积
		 *      f(pj+1 → pj → pj-1) |cosθj|
         *    --------------------------------  
         *             pω(pj+1 - pj)
		 */
		throughput *= f * absDot(wi, isect.shading.normal) / pdf;
		CHECK_GE(throughput.y(), 0.0f);
		DCHECK(!std::isinf(throughput.y()));
		specularBounce = (flags & BSDF_SPECULAR) != 0;

		if ((flags & BSDF_TRANSMISSION)) {
			Float eta = isect.bsdf->eta;
			// 详见bxdf.hpp文件中SpecularTransmission的注释
			etaScale *= (dot(wo, isect.normal) > 0) ? (eta * eta) : 1 / (eta * eta);
		}

		ray = isect.spawnRay(wi);
		if (isect.bssrdf && (flags & BSDF_TRANSMISSION)) {
			// todo 处理bssrdf
		}
		// Spectrum rrThroughput = throughput * etaScale;
		// 可能会由于俄罗斯轮盘而结束当前路径追踪
		if (throughput.MaxComponentValue() < _rrThreshold && bounces > 3) {
            Float q = std::max((Float)0.05, 1 - throughput.MaxComponentValue());
            if (sampler.get1D() < q) {
                break;
			}
			throughput /= 1 - q;
            DCHECK(!std::isinf(throughput.y()));
		}
	}

    return L;
}

USING_STD;

//"param" : {
//    "maxBounce" : 5,
//    "rrThreshold" : 1,
//    "lightSampleStrategy" : "power"
//}
// lst = {sampler, camera}
CObject_ptr createPathTracer(const nebJson &param, const Arguments &lst) {
    int maxBounce = param.GetValue("maxBounce", 5);
    Float rrThreshold = param.GetValue("rrThreshold", 1.f);
    string lightSampleStrategy = param.GetValue("lightSampleStrategy", "power");
    auto iter = lst.begin();
    Sampler * sampler = dynamic_cast<Sampler *>(*iter);
    ++iter;
    Camera * camera = dynamic_cast<Camera *>(*iter);
    AABB2i pixelBounds = camera->film->getSampleBounds();
    PathTracer * ret = new PathTracer(maxBounce,
                                      shared_ptr<const Camera>(camera),
                                      shared_ptr<Sampler>(sampler),
                                      pixelBounds,
                                      rrThreshold,
                                      lightSampleStrategy);
    
    return ret;
}

REGISTER("PathTracer", createPathTracer);



PALADIN_END
