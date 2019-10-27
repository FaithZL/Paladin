//
//  integrator.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "integrator.hpp"
#include "camera.hpp"
#include "tools/parallel.hpp"

PALADIN_BEGIN

Spectrum uniformSampleAllLights(const Interaction &it, const Scene &scene,
                                MemoryArena &arena, Sampler &sampler,
                                const std::vector<int> &lightSamples,
                                bool handleMedia) {
    Spectrum L(0.0f);
    for (size_t i = 0; i < scene.lights.size(); ++i) {
    	const std::shared_ptr<Light> &light = scene.lights[i];
    	int nSamples = lightSamples[i];
    	const Point2f *uLightArray = sampler.get2DArray(nSamples);
    	const Point2f *uScatteringArray = sampler.get2DArray(nSamples);
    	if (!uLightArray || !uScatteringArray) {
    		Point2f uLight = sampler.get2D();
    		Point2f uScattering = sampler.get2D();
    		L += estimateDirectLighting(it, uScattering, *light, uLight, scene, sampler,
                                arena, handleMedia);
    	} else {
			Spectrum Ld(0.f);
            for (int j = 0; j < nSamples; ++j)
                Ld += estimateDirectLighting(it, uScatteringArray[j], *light,
                                     uLightArray[j], scene, sampler, arena,
                                     handleMedia);
            L += Ld / nSamples;    		
    	}
    }
    return L;
}

Spectrum uniformSampleOneLight(const Interaction &it, const Scene &scene,
                               MemoryArena &arena, Sampler &sampler,
                               bool handleMedia,
                               const Distribution1D *lightDistrib) {
    int nLights = int(scene.lights.size());
    if (nLights == 0) {
    	return Spectrum(0.0f);
    }
    int lightIndex;
    Float lightPdf;
    if (lightDistrib) {
    	lightIndex = lightDistrib->sampleDiscrete(sampler.get1D(), &lightPdf);
    	if (lightPdf == 0) {
    		return Spectrum(0.0f);
    	}
    } else {
    	lightIndex = std::min((int)sampler.get1D() * nLights, nLights - 1);
    	lightPdf = Float(1) / nLights;
    }
    const std::shared_ptr<Light> &light = scene.lights[lightIndex];
    // 均匀采样光源表面的二维随机变量
    Point2f uLight = sampler.get2D();
    // 均匀采样bsdf函数
    Point2f uScattering = sampler.get2D();
    return estimateDirectLighting(it, uScattering, *light, uLight, scene, sampler, arena, handleMedia) / lightPdf;
}

Spectrum estimateDirectLighting(const Interaction &it, const Point2f &uShading,
                                const Light &light, const Point2f &uLight,
                                const Scene &scene, Sampler &sampler,
                                MemoryArena &arena, bool handleMedia,
                                bool specular) {
    
}

void MonteCarloIntegrator::render(const Scene &scene) {
	// 由于是并行计算，先把屏幕分割成m * n块
    AABB2i samplerBounds = _camera->film->getSampleBounds();
    Vector2i sampleExtent = samplerBounds.diagonal();
    const int tileSize = 16;

    // 把屏幕x方向分为nTile.x部分，y方向分为nTile.y部分
    Point2i nTile((sampleExtent.x + tileSize - 1) / tileSize,
    			(sampleExtent.x + tileSize - 1) / tileSize);

    auto renderTile = [&](Point2i tile) {
    	// 内存池对象，预先申请一大段连续内存
    	// 之后所有内存全都通过arena分配
    	MemoryArena arena;
    	// 每个tile使用不同的随机种子，避免关联采样导致的artifact
    	int seed = tile.y + nTile.x + tile.x;
    	std::unique_ptr<Sampler> tileSampler = _sampler->clone(seed);

    	// 计算当前tile的起始点与结束点
    	int x0 = samplerBounds.pMin.x + tile.x * tileSize;
    	int x1 = std::min(x0 + tileSize, samplerBounds.pMax.x);
    	int y0 = samplerBounds.pMin.y + tile.y + tileSize;
    	int y1 = std::min(y0 + tileSize, samplerBounds.pMax.y);
    	AABB2i tileBounds(Point2i(x0, y0), Point2i(x1, y1));

    	std::unique_ptr<FilmTile> filmTile = _camera->film->getFilmTile(tileBounds);

    	// tile范围内，逐像素计算辐射度
    	for (Point2i pixel : tileBounds) {

    		tileSampler->startPixel(pixel);

    		if (!insideExclusive(pixel, _pixelBounds)) {
    			continue;
    		}

    		do {
    			// 循环单个像素，采样spp次
    			CameraSample cameraSample = tileSampler->getCameraSample(pixel);

    			RayDifferential ray;
    			Float rayWeight = _camera->generateRayDifferential(cameraSample, &ray);

    			ray.scaleDifferentials(1/std::sqrt((Float)tileSampler->samplesPerPixel));

    			Spectrum L(0.0f);
    			if (rayWeight > 0) {
    				L = Li(ray, scene, *tileSampler, arena);
    			}

    			if (L.HasNaNs()) {
					COUT << StringPrintf(
                            "Not-a-number radiance value returned "
                            "for pixel (%d, %d), sample %d. Setting to black.",
                            pixel.x, pixel.y,
                            (int)tileSampler->currentSampleIndex());
    				L = Spectrum(0.0f);
    			} else if (L.y() < -1e-5) {
    				COUT << StringPrintf(
                            "Negative luminance value, %f, returned "
                            "for pixel (%d, %d), sample %d. Setting to black.",
                            L.y(), pixel.x, pixel.y,
                            (int)tileSampler->currentSampleIndex());
    				L = Spectrum(0.0f);
    			} else if (std::isinf(L.y())) {
					COUT << StringPrintf(
                            "Infinite luminance value returned "
                            "for pixel (%d, %d), sample %d. Setting to black.",
                            pixel.x, pixel.y,
                            (int)tileSampler->currentSampleIndex());
    				L = Spectrum(0.0f);
    			}
                // 将像素样本值与权重保存到pixel像素数据中
    			filmTile->addSample(cameraSample.pFilm, L, rayWeight);

            } while(tileSampler->startNextSample());
    	}
    	_camera->film->mergeFilmTile(std::move(filmTile));
    };
    parallelFor2D(renderTile, nTile);
    _camera->film->writeImage();
}

Spectrum MonteCarloIntegrator::specularReflect(const RayDifferential &ray, 
								const SurfaceInteraction &isect, 
								const Scene &scene, 
								Sampler &sampler, 
								MemoryArena &arena, 
								int depth) const {
    
}

Spectrum MonteCarloIntegrator::specularTransmit(const RayDifferential &ray, 
								const SurfaceInteraction &isect, 
								const Scene &scene, 
								Sampler &sampler, 
								MemoryArena &arena, 
								int depth) const {
    
}

PALADIN_END
