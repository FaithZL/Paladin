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
#include "tools/progressreporter.hpp"

PALADIN_BEGIN

Spectrum uniformSampleAllLights(const Interaction &it, const Scene &scene,
                                MemoryArena &arena, Sampler &sampler,
                                const std::vector<int> &lightSamples,
                                bool handleMedia) {
    Spectrum L(0.0f);
    // 逐个光源遍历，估计直接光照
    for (size_t i = 0; i < scene.lights.size(); ++i) {
    	const std::shared_ptr<Light> &light = scene.lights[i];
    	int nSamples = lightSamples[i];
        // get2DArray获取的是同一维度的nSamples个样本
    	const Point2f *uLightArray = sampler.get2DArray(nSamples);
    	const Point2f *uScatteringArray = sampler.get2DArray(nSamples);
    	if (!uLightArray || !uScatteringArray) {
    		Point2f uLight = sampler.get2D();
    		Point2f uScattering = sampler.get2D();
    		L += estimateDirectLighting(it, uScattering, *light, uLight, scene, sampler,
                                arena, handleMedia);
    	} else {
			Spectrum Ld(0.f);
            // 用get2DArray获取到的一系列样本来采样光源
            for (int j = 0; j < nSamples; ++j) {
                Ld += estimateDirectLighting(it, uScatteringArray[j], *light,
                                     uLightArray[j], scene, sampler, arena,
                                     handleMedia);
            }
            L += Ld / nSamples;    		
    	}
    }
    return L;
}

Spectrum sampleOneLight(const Interaction &it, const Scene &scene,
                               MemoryArena &arena, Sampler &sampler,
                               bool handleMedia,
                               const Distribution1D *lightDistrib) {
    // 与uniformSampleAllLights不同的是，
    // 该函数会按照对应分布随机采样一个光源，这个接口会比较常用，也比较科学
    int nLights = int(scene.lights.size());
    if (nLights == 0) {
    	return Spectrum(0.0f);
    }
    int lightIndex;
    // 用于储存选中的光源的概率密度函数值
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
    // 估计当前选中的光源对该点的辐射度
    Spectrum dl = estimateDirectLighting(it, uScattering, *light, uLight, scene, sampler, arena, handleMedia);
    // 需要注意的是！返回值为dl / lightPdf
    // 表示对整个场景中的所有光源对it的直接光照估计
    return  dl / lightPdf;
}

Spectrum estimateDirectLighting(const Interaction &it, const Point2f &uScattering,
                                const Light &light, const Point2f &uLight,
                                const Scene &scene, Sampler &sampler,
                                MemoryArena &arena, bool handleMedia,
                                bool specular) {
    // 这个函数比较长，简述一下基本思路
    // 先随机采样光源表面，生成light光源表面点P1，计算从P1发射的光在it点产生的辐射度L1
    // 再随机采样it处的bsdf，生成一个ray，如果顺着ray方向能找到光P1点所在的光源
    // 则计算出ray与光源表面的交点P2，计算从P2发射的光线在it点产生的辐射度L2
    // 根据复合重要性采样的公式，估计出it受到light的直接光照
    // 如果采样bsdf时生成的ray不与light相交，则返回对L1加权之后的辐射度
    // 具体逻辑，看代码
    BxDFType bsdfFlags = specular ?
    					BSDF_ALL : 
    					BxDFType(BSDF_ALL & ~BSDF_SPECULAR);
    Spectrum Ld(0.0f);
    Float lightPdf = 0;
    Vector3f wi;
    Float scatteringPdf = 0;
    VisibilityTester visibility;
    // 先采样光源表面
    Spectrum Li = light.sample_Li(it, uLight, &wi, &lightPdf, &visibility);
    if (lightPdf > 0 && !Li.IsBlack()) {
    	Spectrum f;
    	// 为当前光源的样本计算bsdf
    	if (it.isSurfaceInteraction()) {
    		const SurfaceInteraction &isect = (const SurfaceInteraction &)it;
    		f = isect.bsdf->f(isect.wo, wi, bsdfFlags) * absDot(wi, isect.shading.normal);
            scatteringPdf = isect.bsdf->pdfDir(isect.wo, wi, bsdfFlags);
    	} else {
            const MediumInteraction &mi = (const MediumInteraction &)it;
            Float p = mi.phase->p(mi.wo, wi);
            f = Spectrum(p);
            scatteringPdf = p;
    	}
    	if (!f.IsBlack()) {
    		// 计算可见性
    		if (handleMedia) {
                Li *= visibility.Tr(scene, sampler);
    		} else {
    			if (!visibility.unoccluded(scene)) {
    				Li = Spectrum(0.0f);
    			}
    		}
			if (!Li.IsBlack()) {
				// 如果是delta分布，直接计算辐射度
	    		if (light.isDelta()) {
	    			Ld += f * Li / lightPdf;
	    		} else {
	    			// 非delta分布，用复合重要性采样
	    			Float weight = powerHeuristic(1, lightPdf, 1, scatteringPdf);
	    			Ld += f * Li * weight / lightPdf;
	    		}
	    	}
    	}
    }

    // 对bsdf进行随机采样
    if (!light.isDelta()) {
    	Spectrum f;
    	bool sampledSpecular = false;
    	if (it.isSurfaceInteraction()) {
    		BxDFType sampledType;
    		const SurfaceInteraction &isect = (const SurfaceInteraction &)it;
            f = isect.bsdf->sample_f(isect.wo, &wi, uScattering, 
            			&scatteringPdf, bsdfFlags, &sampledType);
            f *= absDot(wi, isect.shading.normal);
    		sampledSpecular = (sampledType & BSDF_SPECULAR) != 0;
    	} else {
    		const MediumInteraction &mi = (const MediumInteraction &)it;
            Float p = mi.phase->sample_p(mi.wo, &wi, uScattering);
            f = Spectrum(p);
            scatteringPdf = p;
    	}

        if (!f.IsBlack() && scatteringPdf > 0) {
            // 为何高光采样权重就是1？
            // 因为如果是高光，scatteringPdf实际上应为正无穷
            // weight自然也是正无穷
            // 所以特殊处理之后weight与scatteringPdf都取1
            Float weight = 1;
            // 如果采集到的样本不是高光反射，则修改权重
            if (!sampledSpecular) {
                lightPdf = light.pdf_Li(it, wi);
                if (lightPdf == 0) {
                    return Ld;
                }
                weight = powerHeuristic(1, scatteringPdf, 1, lightPdf);
            }

            SurfaceInteraction lightIsect;
            Ray ray = it.spawnRay(wi);
            Spectrum Tr(1.0f);
            
            bool foundSurfaceInteraction = handleMedia
                            ? scene.intersectTr(ray, sampler, &lightIsect, &Tr)
                            : scene.intersect(ray, &lightIsect);
            Spectrum Li(0.0f);
            if (foundSurfaceInteraction) {
                // 如果找到的交点是light光源上的点，则计算光照
                if (lightIsect.primitive->getAreaLight() == &light) {
                    Li = lightIsect.Le(-wi);
                }
            } else {
                // 如果没有交点，Li为0，这里写得不是很好todo
                Li = light.Le(ray);
            }
            if (!Li.IsBlack()) {
                Ld += f * Li * Tr * weight / scatteringPdf;
            }
        }
    }
    return Ld;
}

void MonteCarloIntegrator::render(const Scene &scene) {
    preprocess(scene, *_sampler);
    
	// 由于是并行计算，先把屏幕分割成m * n块
    AABB2i samplerBounds = _camera->film->getSampleBounds();
    Vector2i sampleExtent = samplerBounds.diagonal();
    const int tileSize = 16;

    // 把屏幕x方向分为nTile.x部分，y方向分为nTile.y部分
    Point2i nTile((sampleExtent.x + tileSize - 1) / tileSize,
    			(sampleExtent.y + tileSize - 1) / tileSize);
    
    outputSceneInfo(scene);
    
    ProgressReporter reporter("rendering", nTile.x * nTile.y);
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
    	int y0 = samplerBounds.pMin.y + tile.y * tileSize;
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
                    DCHECK(false);
    				L = Spectrum(0.0f);
    			} else if (L.y() < -1e-5) {
    				COUT << StringPrintf(
                            "Negative luminance value, %f, returned "
                            "for pixel (%d, %d), sample %d. Setting to black.",
                            L.y(), pixel.x, pixel.y,
                            (int)tileSampler->currentSampleIndex());
                    DCHECK(false);
    				L = Spectrum(0.0f);
    			} else if (std::isinf(L.y())) {
					COUT << StringPrintf(
                            "Infinite luminance value returned "
                            "for pixel (%d, %d), sample %d. Setting to black.",
                            pixel.x, pixel.y,
                            (int)tileSampler->currentSampleIndex());
                    DCHECK(false);
    				L = Spectrum(0.0f);
    			}
                
                // 将像素样本值与权重保存到pixel像素数据中
    			filmTile->addSample(cameraSample.pFilm, L, rayWeight);
                arena.reset();
            } while (tileSampler->startNextSample());
    	}
        reporter.update();
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
    Vector3f wo = isect.wo;
    Vector3f wi;
    Float pdf;
    BxDFType type = BxDFType(BSDF_REFLECTION | BSDF_SPECULAR);
    // 随机采样与type类型符合的bxdf，生成对应方向，返回该方向上的bsdf函数值
    Spectrum f = isect.bsdf->sample_f(wo, &wi, sampler.get2D(), &pdf, type);
    const Normal3f &ns = isect.shading.normal;

    if (pdf > 0.0f && !f.IsBlack() && absDot(wi, ns) != 0.0f) {
        // 生成wi方向的主光线
        RayDifferential rd = isect.spawnRay(wi);
        if (ray.hasDifferentials) {
            rd.hasDifferentials = true;
            // 计算反射差分光线的起点，这还是比较简单的
            rd.rxOrigin = isect.pos + isect.dpdx;
            rd.ryOrigin = isect.pos + isect.dpdy;
            /**
             * 计算反射差分光线的方向
             * 用正向差分法去近似，表达式如下
             * ω ≈ ωi + dωi/dx     0式
             * 由反射向量公式 ωi = 2(ωo · n)n - ωo
             * 
             *  dωi     d(2(ωo · n)n - ωo)
             * ----- = --------------------     1式
             *  dx             dx
             * 
             *                dn     d(ωo · n)          dωo
             * = 2 [(ωo · n) ---- + ----------- n]  -  -----  2式
             *                dx        dx              dx
             * 其中 
             *  d(ωo · n)     dωo              dn
             * ----------- = ----- · n + ωo · ----  3式
             *     dx         dx               dx
             * 以上表达式的推导只用到了高中数学学过的乘法法则，很容易推导出来
             * 以后如果实在不想动手，推荐一个工具wolframalpha，堪称神器！
             */
            // 复合函数求导，链式法则不解释！
            Normal3f dndx = isect.shading.dndu * isect.dudx
                          + isect.shading.dndv * isect.dvdx;
            // 注意出射方向的定义ray.rxDirection要乘以-1
            Vector3f dwodx = -ray.rxDirection - wo;
            // 3式
            Float dDNdx = dot(dwodx, ns) + dot(wo, dndx);
            // 2式结合0式
            rd.rxDirection = wi - dwodx 
            		+ 2.f * Vector3f(dot(wo, ns) * dndx + dDNdx * ns);
            // y方向的差分光线的方向求法同上
            Normal3f dndy = isect.shading.dndu * isect.dudy
                          + isect.shading.dndv * isect.dvdy;
            Vector3f dwody = -ray.ryDirection - wo;
            Float dDNdy = dot(dwody, ns) + dot(wo, dndy);
			rd.ryDirection = wi - dwody
				    + 2.f * Vector3f(dot(wo, ns) * dndy + dDNdy * ns);
        }
        return f * Li(rd, scene, sampler, arena, depth + 1) * absDot(wi, ns) / pdf;
    } else {
    	return Spectrum(0.0f);
    }
}

Spectrum MonteCarloIntegrator::specularTransmit(const RayDifferential &ray, 
								const SurfaceInteraction &isect, 
								const Scene &scene, 
								Sampler &sampler, 
								MemoryArena &arena, 
								int depth) const {
    Vector3f wo = isect.wo;
    Vector3f wi;
    Float pdf;
    const Point3f pos;
    const BSDF &bsdf = *isect.bsdf;
    BxDFType type = BxDFType(BSDF_TRANSMISSION | BSDF_SPECULAR);
    Spectrum f = bsdf.sample_f(wo, &wi, sampler.get2D(), &pdf, type);
    Spectrum L(0.0f);
    Normal3f ns = isect.shading.normal;

    if (pdf > 0.0f && !f.IsBlack() && absDot(wi, ns) != 0) {
    	RayDifferential rd = isect.spawnRay(wi);
    	if (ray.hasDifferentials) {
    		rd.hasDifferentials = true;
    		rd.rxOrigin = pos + isect.dpdx;
    		rd.ryOrigin = pos + isect.dpdy;

			Normal3f dndx = isect.shading.dndu * isect.dudx +
                            isect.shading.dndv * isect.dvdx;
            Normal3f dndy = isect.shading.dndu * isect.dudy +
                            isect.shading.dndv * isect.dvdy;
            // 假设光线进入物体
            Float eta = 1 / bsdf.eta;
            if (dot(wo, ns) < 0) {
            	eta = 1 / eta;
            	ns = -ns;
            	dndx = -dndx;
            	dndy = -dndy;
            }

            /**
             * 由折射公式
             * η = ηi/ηt
             * ωt = -η ωi + [η (ωi · n) - cosθt] n
             *
             *  dωt		d(-η ωi + [η (ωi · n) - cosθt] n)
             * ----- = ----------------------------------
             *  dx                   dx
             *
             *     d(-η ωi)     d(η (ωi · n) n)     d(n cosθt)
             * = ----------- + ---------------- - ------------
             *       dx            dx                 dx
             */
			Vector3f dwodx = -ray.rxDirection - wo,
                     dwody = -ray.ryDirection - wo;
            Float dDNdx = dot(dwodx, ns) + dot(wo, dndx);
            Float dDNdy = dot(dwody, ns) + dot(wo, dndy);

            Float mu = eta * dot(wo, ns) - absDot(wi, ns);
            Float dmudx =
                (eta - (eta * eta * dot(wo, ns)) / absDot(wi, ns)) * dDNdx;
            Float dmudy =
                (eta - (eta * eta * dot(wo, ns)) / absDot(wi, ns)) * dDNdy;

            rd.rxDirection =
                wi - eta * dwodx + Vector3f(mu * dndx + dmudx * ns);
            rd.ryDirection =
                wi - eta * dwody + Vector3f(mu * dndy + dmudy * ns);            
    	}
    	L = f * Li(rd, scene, sampler, arena, depth + 1) * absDot(wi, ns) / pdf;
    }
    return L;
}

PALADIN_END
