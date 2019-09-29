//
//  texture.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef texture_hpp
#define texture_hpp

#include "header.h"
#include "interaction.hpp"

PALADIN_BEGIN

/**
 * 纹理基类
 * 图像坐标为(x,y)，纹理坐标为(s,t)
 * 
 * 假设纹理函数为f
 * 我们采样一个纹理的值是不需要计算任意的f(x,y)的值
 * 我们只需要找到特定的点上，像素样本的变化与纹理样本变换的关系
 * 
 * f(x',y') ≈ f(x,y) + df/dx * (x' - x) + df/dy * (y' - y)
 * 
 */
template <typename T>
class Texture {
public:
    virtual T evaluate(const SurfaceInteraction &) const = 0;
    
    virtual ~Texture() {
        
    }
};

PALADIN_END

#endif /* texture_hpp */
