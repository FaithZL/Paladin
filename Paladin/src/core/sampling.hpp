//
//  sampling.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef sampling_hpp
#define sampling_hpp

#include "header.h"

/*
P为累积分布函数(cdf)，p为概率密度函数(pdf)，cdf求导得到pdf
常用概率论基础公式

Pr{X ≤ x} = Pr{Y ≤ y(x)}
Py = Py(y(x)) = Px
两边积分得 

py(y) * dy/dx = px(x)  1式

dw = sinθdθdφ   2式

p(θ, φ)dθdφ = p(w)dw   3式

p(θ, φ) = sinθ p(w)  4式

如果X,Y两个随机变量相互独立
p(x, y) = px(x) * py(y)  5式
 */

#endif /* sampling_hpp */
