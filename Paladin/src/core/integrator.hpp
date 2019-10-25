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

/**
 * 先简单介绍一下积分器
 * 所有渲染的问题本质上是求解积分的问题
 * 场景文件中记录着场景中的光源数据，以及几何信息，每个表面的材质，相机的参数等等
 * 
 * 渲染也可以理解为计算几何表面上某个点在对应像素上的颜色贡献。
 * 
 * 渲染方程如下所示
 *    Lo(p, ωo) = Le(p, ωo) + ∫[hemisphere]f(p, ωi, ωo)Li(p, ωi)|cosθi|dωi   1式
 * 
 * Lo(p, ωo)为p点向ωo方向贡献的辐射度
 * 
 * 假设场景中没有参与介质，辐射度在传播过程中不会改变，则
 * 
 *    Li(p, ω) = Lo(t(p, ω), -ω)    2式
 *
 * 其中t(p, ω)为，在p点沿着ω方向上找到的第一个点p'
 *
 * 将1式带入2式得
 *
 *   Lo(p, ωo) = Le(p, ωo) + ∫[hemisphere]f(p, ωi, ωo)Lo(t(p, ωi), -ωi)|cosθi|dωi
 * 
 *
 *
 */
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
