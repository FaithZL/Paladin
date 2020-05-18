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
#include "core/primitive.hpp"
#include "core/light.hpp"
#include "tools/embree_util.hpp"
#include "shapes/trianglemesh.hpp"
#include <set>

PALADIN_BEGIN

class Scene {
public:
    
    Scene(const std::vector<std::shared_ptr<Light>> &lights)
    : lights(lights),
    _aggregate(nullptr),
    _rtcScene(nullptr) {
        
    }
    
    Scene(RTCScene rtcScene, const std::vector<std::shared_ptr<Light>> &lights)
    : lights(lights),
    _aggregate(nullptr),
    _rtcScene(rtcScene) {
        
    }
    
    void initEnvmap();
    
    void initAccel(const nloJson &data,
                   const vector<shared_ptr<Primitive>> &primitive);
    
    void initAccel(const nloJson &data,
                   const vector<shared_ptr<const Shape>> &shape);

    const AABB3f &worldBound() const { 
        return _worldBound;
    }

    
    F_INLINE bool rayIntersect(const Ray &ray, SurfaceInteraction *isect) const {
        return _rtcScene ?
            rayIntersectEmbree(ray, isect):
            rayIntersectNative(ray, isect);
    }
    
    F_INLINE bool rayOccluded(const Ray &ray) const {
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
    
    F_INLINE bool rayIntersectEmbree(const Ray &ray, SurfaceInteraction *isect) const {
        RTCIntersectContext context;
        rtcInitIntersectContext(&context);
        RTCRayHit rh = EmbreeUtil::toRTCRayHit(ray);
        rtcIntersect1(_rtcScene, &context, &rh);
        if (rh.hit.geomID == RTC_INVALID_GEOMETRY_ID) {
            return false;
        }
        uint32_t gid = rh.hit.geomID;
        uint32_t pid = rh.hit.primID;
        
        
    }
    
    F_INLINE bool rayOccludedEmbree(const Ray &ray) const {
        using namespace EmbreeUtil;
        RTCIntersectContext context;
        rtcInitIntersectContext(&context);
        RTCRay r = EmbreeUtil::convert(ray);
        rtcOccluded1(_rtcScene, &context, &r);
        return r.tfar < 0;
    }
    
    /**
     * 光线在场景中传播的函数
     * @param  ray     指定的光线对象
     * @param  sampler 采样器
     * @param  isect   指定物体表面交点
     * @param  Tr      可以理解为传播的百分比
     * @return         返回ray与isect是否有交点
     */
    bool rayIntersectTr(Ray ray, Sampler &sampler, SurfaceInteraction *isect,
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
    
    vector<shared_ptr<const Shape>> _shapes;
    
    // 片段的集合
    std::shared_ptr<Primitive> _aggregate;
    
    RTCScene _rtcScene;
};

PALADIN_END

#endif /* scene_hpp */
