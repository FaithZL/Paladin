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


PALADIN_BEGIN

class Scene {
public:
    Scene(std::shared_ptr<Primitive> aggregate,
          const std::vector<std::shared_ptr<Light>> &lights)
    : lights(lights), _aggregate(aggregate) {
        _worldBound = _aggregate->worldBound();
        for (const auto &light : lights) {
            light->preprocess(*this);
            if (light->flags & (int)LightFlags::Infinite) {
                infiniteLights.push_back(light);
            }
        }
    }

    const AABB3f &worldBound() const { 
        return _worldBound;
    }

    bool intersect(const Ray &ray, SurfaceInteraction *isect) const {
        CHECK_NE(ray.dir, Vector3f(0,0,0));
        return _aggregate->intersect(ray, isect);
    }

    bool intersectP(const Ray &ray) const {
        CHECK_NE(ray.dir, Vector3f(0,0,0));
        return _aggregate->intersectP(ray);
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
    
    std::vector<std::shared_ptr<Light>> lights;

    std::vector<std::shared_ptr<Light>> infiniteLights;
    
private:
    // 片段的集合
    std::shared_ptr<Primitive> _aggregate;
    // 整个场景的包围盒
    AABB3f _worldBound;
};

PALADIN_END

#endif /* scene_hpp */
