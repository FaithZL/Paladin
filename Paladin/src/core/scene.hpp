//
//  scene.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef scene_hpp
#define scene_hpp

#include "core/header.h"
#include "core/aggregate.hpp"
#include "core/light.hpp"
#include "tools/embree_util.hpp"
#include "lights/envmap.hpp"
#include "tools/stats.hpp"
#include "bxdf.hpp"

PALADIN_BEGIN

STAT_COUNTER("Intersections/Regular ray intersection tests",
             nIntersectionTests);
STAT_COUNTER("Intersections/Shadow ray intersection tests", nShadowTests);

class Scene {
public:
    
    Scene(const std::vector<std::shared_ptr<Light>> &lights)
    : lights(lights),
    _aggregate(nullptr),
    _rtcScene(nullptr),
    _envmap(nullptr) {
        
    }
    
    void initInfiniteLights();
    
    void initAccel(const nloJson &data,
                   const vector<shared_ptr<const Shape>> &shape);

    const AABB3f &worldBound() const { 
        return _worldBound;
    }
    
    Spectrum sampleLightDirect(DirectSamplingRecord *rcd,
                               const Point2f _u,
                               const Distribution1D * lightDistrib,
                               Float *pmf) const;
    
    F_INLINE bool rayIntersect(const Ray &ray, SurfaceInteraction *isect) const {
        TRY_PROFILE(Prof::sceneRayIntersect)
        ++nIntersectionTests;
        return _rtcScene ?
                rayIntersectEmbree(ray, isect):
                rayIntersectNative(ray, isect);
    }
    
    F_INLINE bool rayOccluded(const Ray &ray) const {
        TRY_PROFILE(Prof::sceneRayOccluded)
        ++nShadowTests;
        return _rtcScene ?
                rayOccludedEmbree(ray):
                rayOccludedNative(ray);
    }
    
    F_INLINE bool rayIntersectNative(const Ray &ray, SurfaceInteraction *isect) const {
        return _aggregate->rayIntersect(ray, isect);
    }
    
    F_INLINE bool rayOccludedNative(const Ray &ray) const {
        return _aggregate->rayOccluded(ray);
    }
    
    bool rayIntersectEmbree(const Ray &ray, SurfaceInteraction *isect) const;
    
    F_INLINE bool rayOccludedEmbree(const Ray &ray) const {
        using namespace EmbreeUtil;
        RTCIntersectContext context;
        rtcInitIntersectContext(&context);
        RTCRay r = EmbreeUtil::convert(ray);
        rtcOccluded1(_rtcScene, &context, &r);
        return r.tfar < 0;
    }
    
    F_INLINE Spectrum evalEnvironment(const Ray &ray, Float *pdf,
                         const Distribution1D * lightDistrib) const {
        if (!_envmap) {
            *pdf = 0;
            return 0;
        }
        Float pmf = lightDistrib->discretePDF(_envIndex);
        auto ret = _envmap->evalEnvironment(ray, pdf);
        *pdf *= pmf;
        return ret;
    }
    
    Spectrum sampleOneLight(ScatterSamplingRecord *scatterRcd,
                            MemoryArena &arena,
                            const Distribution1D *lightDistrib,
                            bool *foundIntersect,
                            Spectrum * throughput,
                            bool handleMedia = false) const;
    
    Spectrum estimateDirectLighting(ScatterSamplingRecord *scatterRcd,
                            MemoryArena &arena,
                            const Light &light,
                            DirectSamplingRecord *rcd,
                            const Point2f &u,
                            bool *foundIntersect,
                            Spectrum * throughput,
                            bool handleMedia = false) const;
    
    
    /**
     * 光线在场景中传播的函数
     * @param  ray     指定的光线对象
     * @param  sampler 采样器
     * @param  isect   指定物体表面交点
     * @param  Tr      可以理解为传播的百分比
     * @return         返回ray与isect是否有交点
     */
    bool rayIntersectTr(const Ray &ray, Sampler &sampler, SurfaceInteraction *isect,
                     Spectrum *Tr) const;
    
    std::vector<std::shared_ptr<Light>> lights;

    std::vector<std::shared_ptr<Light>> infiniteLights;
    
    inline RTCScene getRTCScene() const {
        return _rtcScene;
    }
    
    void InitAccelNative(const nloJson &data,
                         const vector<shared_ptr<const Shape>>&shapes);
    
    
    void InitAccelEmbree(const vector<shared_ptr<const Shape>>&shapes);
    
    
private:
    // 整个场景的包围盒
    AABB3f _worldBound;
    
    EnvironmentMap * _envmap;
    
    int _envIndex = -1;
    
    vector<shared_ptr<const Shape>> _shapes;
    
    // 片段的集合
    std::shared_ptr<Aggregate> _aggregate;
    
    RTCScene _rtcScene;
};

PALADIN_END

#endif /* scene_hpp */
