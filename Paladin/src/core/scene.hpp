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
        _worldBound = _aggregate->worldBound();
        for (const auto &light : lights) {
            light->preprocess(*this);
            if (light->flags & (int)LightFlags::Infinite) {
                infiniteLights.push_back(light);
            }
        }
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
    
    inline void accelInitNative(const nloJson &data,
                                const vector<shared_ptr<Primitive> >&primitives) {
        _aggregate = createAccelerator(data, primitives);
    }
    
    void initAccel(const nloJson &data,
                   const vector<shared_ptr<Primitive>> &primitive);

    const AABB3f &worldBound() const { 
        return _worldBound;
    }

    bool intersect(const Ray &ray, SurfaceInteraction *isect) const;
    
    bool embreeIntersect(const Ray &ray, SurfaceInteraction *isect) const;

    bool intersectP(const Ray &ray) const;
    
    bool embreeIntersectP(const Ray &ray) const;
    
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
    
    std::vector<std::shared_ptr<Light>> lights;

    std::vector<std::shared_ptr<Light>> infiniteLights;
    
    inline RTCScene getRTCScene() const {
        return _rtcScene;
    }
    
    void accelInitEmbree(const vector<shared_ptr<Primitive>>&);
    
    bool add(TriangleMesh * mesh) {
        _meshes[mesh] = true;
    }
    
    bool has(TriangleMesh * mesh) const {
        auto iter = _meshes.find(mesh);
        bool ret = iter != _meshes.end();
        return ret;
    }
    
    EmbreeUtil::EmbreeGeomtry * getEmbreeGeomtry(int geomID, int primID) const;
    
    Primitive * getPrimitive(int geomID, int primID) const;
    
private:
    // 片段的集合
    std::shared_ptr<Primitive> _aggregate;
    // 整个场景的包围盒
    AABB3f _worldBound;
    
    vector<EmbreeUtil::EmbreeGeomtry *> _embreeGeometries;
    
    std::map<TriangleMesh *, bool> _meshes;
    
    RTCScene _rtcScene;
};

PALADIN_END

#endif /* scene_hpp */
