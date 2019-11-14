//
//  mitchell.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/9/13.
//

#ifndef mitchell_hpp
#define mitchell_hpp

#include "core/filter.h"
#include "tools/classfactory.hpp"

PALADIN_BEGIN

/**
 * 同高斯滤波器一样，分解成xy两个维度，分别进行计算
 * Mitchell一维函数是一个偶函数有效区间是[-2,2]，并且是一个分段函数
 * 
 * 			((12-9B-6C) * |x|^3 + (-18+12B+6C) * |x|^2 + (6-2B)) / 6   		when |x| < 1
 * f(x) = 	((-B-6C) * |x|^3 + (6B+30C) * |x|^2 + (-12B-48C) * |x|) / 6		when 1 ≤ |x| < 2
 * 			0 	when other
 * 
 * 同高斯滤波器一样，有较好的过滤结果
 */
class MitchellFilter : public Filter {
    
public:
    
    MitchellFilter(const Vector2f r, Float B, Float C):
    Filter(r),
    _B(B),
    _C(C) {
        
    }
    
    Float evaluate(const Point2f &p) const {
        return mitchell1D(p.x * invRadius.x) * mitchell1D(p.y * invRadius.y);
    }
    
    Float mitchell1D(Float x) const {
        // todo 这里可以优化一下,用现成的0.1666666666代替 1/6
        x = std::abs(2 * x);
        if (x > 1)
            return ((-_B - 6 * _C) * x * x * x + (6 * _B + 30 * _C) * x * x +
                    (-12 * _B - 48 * _C) * x + (8 * _B + 24 * _C)) *
            (1.f / 6.f);
        else
            return ((12 - 9 * _B - 6 * _C) * x * x * x +
                    (-18 + 12 * _B + 6 * _C) * x * x + (6 - 2 * _B)) *
            (1.f / 6.f);
    }
    
private:

    const Float _B;
    const Float _C;
};

USING_STD

shared_ptr<Serializable> createMitchellFilter(const nebJson &param);

PALADIN_END

#endif /* mitchell_hpp */
