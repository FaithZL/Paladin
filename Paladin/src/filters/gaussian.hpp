//
//  gaussian.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/14.
//

#ifndef gaussian_hpp
#define gaussian_hpp

#include <stdio.h>
#include "tools/classfactory.hpp"
#include "core/filter.h"

PALADIN_BEGIN

/**
 * 不像盒式滤波器跟三角滤波器
 * 高斯滤波器可以得出一个比较好的结果
 *
 * 把像素中心与高斯分布(正态分布)函数的最高点相对应
 * 一维高斯函数如下
 * g(x) = e^(-α * x * x)，由于正态分布函数恒大于零
 * 如果超过r的部分需要为零的话，g(x)必须往下移动，减去一个常量c
 * 显然，x = r, f(x) = 0，可以看出常量c = e^(-α * r * r)
 *
 * 可以得出高斯滤波函数为
 *
 * f(x) = g(x) - c = e^(-α * x * x) - e^(-α * r * r)
 * α越小，函数变化得越缓慢
 * 可以看出，当x=0时，f(x) = 1，符合我们的预期
 *
 * 二维高斯函数就是两个高斯函数的乘积
 */
class GaussianFilter : public Filter {
    
public:
    
    GaussianFilter(const Vector2f r, Float alpha):
    Filter(r),
    _alpha(alpha),
    _expX(std::exp(-_alpha * radius.x * radius.x)),
    _expY(std::exp(-_alpha * radius.y * radius.y)) {
        
    }
    
    // todo 看看这里是否需要优化
    virtual Float evaluate(const Point2f &p) const {
        return gaussian(p.x, _expX) * gaussian(p.y, _expY);
    }
    
private:
    
    const Float _alpha;
    
    const Float _expX, _expY;
    
    Float gaussian(Float d, Float expv) const {
        return std::max((Float)0, Float(std::exp(-_alpha * d * d) - expv));
    }
    
};

USING_STD
CObject_ptr createGaussianFilter(const nloJson &);

PALADIN_END

#endif /* gaussian_hpp */
