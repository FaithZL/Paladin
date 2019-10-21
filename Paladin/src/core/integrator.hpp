//
//  integrator.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef integrator_hpp
#define integrator_hpp

#include "header.h"
#include "scene.hpp"
#include "primitive.hpp"
#include "spectrum.hpp"
#include "light.hpp"
#include "bxdf.hpp"
#include "sampler.hpp"
#include "material.hpp"

PALADIN_BEGIN

class Integrator {
    
public:
    virtual ~Integrator() {
        
    }
    virtual void render(const Scene &) = 0;
};

/**
 * 均匀采样所有光源
 * @param  it            场景中的点
 * @param  scene         场景对象
 * @param  arena         内存池
 * @param  sampler       采样器
 * @param  nLightSamples 光源样本数量列表
 * @param  handleMedia   是否处理参与介质
 * @return               辐射度
 */
Spectrum uniformSampleAllLights(const Interaction &it, const Scene &scene,
                                MemoryArena &arena, Sampler &sampler,
                                const std::vector<int> &nLightSamples,
                                bool handleMedia = false);

/**
 * 按照指定分布随机采样一个光源
 * @param  it           场景中的点
 * @param  scene        场景对象
 * @param  arena        内存池
 * @param  sampler      采样器
 * @param  handleMedia  是否处理参与介质
 * @param  lightDistrib 光源分布
 * @return              辐射度
 */
Spectrum uniformSampleOneLight(const Interaction &it, const Scene &scene,
                               MemoryArena &arena, Sampler &sampler,
                               bool handleMedia = false,
                               const Distribution1D *lightDistrib = nullptr);

/**
 * 用复合重要性采样进行直接光照的估计
 * @param  it          场景中的点
 * @param  uShading    用于采样着色点的随机变量，用于生成wi方向，采样bsdf
 * @param  light       光源对象
 * @param  uLight      用于采样光源表面的二维随机变量
 * @param  scene       场景对象
 * @param  sampler     采样器
 * @param  arena       内存池
 * @param  handleMedia 是否处理参与介质
 * @param  specular    是否为高光
 * @return             返回直接光照辐射度
 */
Spectrum estimateDirectLighting(const Interaction &it, const Point2f &uShading,
                        const Light &light, const Point2f &uLight,
                        const Scene &scene, Sampler &sampler,
                        MemoryArena &arena, bool handleMedia = false,
                        bool specular = false);

PALADIN_END

#endif /* integrator_hpp */
