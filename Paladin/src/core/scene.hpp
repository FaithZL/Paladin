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
    Scene(std::shared_ptr<Primitive> aggregate,
          const std::vector<std::shared_ptr<Light>> &lights)
    : lights(lights),
    _aggregate(aggregate),
    _rtcScene(nullptr) {
        
    }
    
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
    
    inline void accelInitNative(const nloJson &data,
                                const vector<shared_ptr<Primitive> >&primitives);
    
    void initAccel(const nloJson &data,
                   const vector<shared_ptr<Primitive>> &primitive);
    
    void initAccel(const nloJson &data,
                   const vector<shared_ptr<Shape>> &shape);

    const AABB3f &worldBound() const { 
        return _worldBound;
    }

    bool intersect(const Ray &ray, SurfaceInteraction *isect) const;
    
    bool embreeIntersect(const Ray &ray, SurfaceInteraction *isect) const;

    bool intersectP(const Ray &ray) const;
    
    bool embreeIntersectP(const Ray &ray) const;
    
    
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
        return _prims->rayIntersect(ray, isect);
    }
    
    F_INLINE bool rayOccludedNative(const Ray &ray) const {
        return _prims->rayOccluded(ray);
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
        // TODO
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
    bool intersectTr(Ray ray, Sampler &sampler, SurfaceInteraction *isect,
                     Spectrum *Tr) const;
    
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
                         const vector<shared_ptr<Shape>>&shapes);
    
    void accelInitEmbree(const vector<shared_ptr<Primitive>>&);
    
    void InitAccelEmbree(const vector<shared_ptr<Shape>>&shapes);
    
    bool add(TriangleMesh * mesh) {
        _meshes[mesh] = true;
    }
    
    bool has(TriangleMesh * mesh) const {
        auto iter = _meshes.find(mesh);
        bool ret = iter != _meshes.end();
        return ret;
    }
    
    EmbreeUtil::EmbreeGeomtry * getEmbreeGeomtry(int geomID) const;
    
    Primitive * getPrimitive(int geomID, int primID) const;
    
private:
    // 片段的集合
    std::shared_ptr<Primitive> _aggregate;
    // 整个场景的包围盒
    AABB3f _worldBound;
    
    // 片段的集合
    std::shared_ptr<Primitive> _prims;
    
    vector<EmbreeUtil::EmbreeGeomtry *> _embreeGeometries;
    
    std::map<TriangleMesh *, bool> _meshes;
    
    RTCScene _rtcScene;
};

PALADIN_END

#endif /* scene_hpp */
