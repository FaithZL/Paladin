//
//  medium.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/7/16.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef medium_hpp
#define medium_hpp

#include "core/header.h"

PALADIN_BEGIN

// 正如描述表面散射的BSDF模型种类繁多，许多相函数也得到了发展。
// 这些模型从参数化模型(可用于将具有少量参数的函数拟合到测量数据)
// 到基于从具有已知形状和材料的粒子(如球形水滴)推导出散射辐射分布的分析模型。
// 相函数是一个一维函数，参数为cosθ，其中θ为入射方向与出射方向的夹角
// 相函数有互换性，任意交换两个向量，函数值保持不变
// 相函数满足以下表达式
//       ∫[sphere]p(ωo,ωi) = 1
// 所以各向同性的相函数满足
// p(ωo,ωi) = 1/4π
class PhaseFunction {
public:
   
    virtual ~PhaseFunction() {

    }

    virtual Float p(const Vector3f &wo, const Vector3f &wi) const = 0;

    virtual Float sample_p(const Vector3f &wo, Vector3f *wi,
                           const Point2f &u) const = 0;

    virtual std::string toString() const = 0;
};

// 介质
class Medium {
public:
    // Medium Interface
    virtual ~Medium() {}
    virtual Spectrum tr(const Ray &ray, Sampler &sampler) const = 0;
    virtual Spectrum sample(const Ray &ray, Sampler &sampler,
                            MemoryArena &arena,
                            MediumInteraction *mi) const = 0;
};


// 两个介质的相交处，nullptr表示真空
struct MediumInterface {
    MediumInterface() : inside(nullptr), outside(nullptr) {}

    MediumInterface(const Medium *medium) : inside(medium), outside(medium) {

    }

    MediumInterface(const Medium *inside, const Medium *outside)
    :inside(inside),
    outside(outside) {

    }
    
    bool isMediumTransition() const {
        return inside != outside;
    }
    // 内部的介质
    const Medium *inside;
    
    // 外部的介质
    const Medium *outside;
};

PALADIN_END

#endif /* medium_hpp */
