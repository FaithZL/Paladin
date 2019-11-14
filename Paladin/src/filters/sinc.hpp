//
//  sinc.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/9/13.
//

#ifndef sinc_hpp
#define sinc_hpp

#include "core/filter.h"

PALADIN_BEGIN

/**
 * LanczosSinc是由sinc函数经过改良之后得到的
 *
 * sinc函数为 sinc(x) = sin(πx)/(πx)
 * 
 * LanczosSinc函数为sinc函数乘以一个Lanczos window函数得到
 *
 * LanczosSinc window函数为sinc函数的周期扩大τ倍(注意，这里的周期是指函数曲线与x轴的相邻两个交点的距离)
 * 
 * w(x) = sinc(x/τ)
 * 思路已经很清晰了，代码如下
 */
class LanczosSincFilter : public Filter {
    
public:
    LanczosSincFilter(const Vector2f r, Float tau):
    Filter(r),
    _tau(tau) {
        
    }
    
    virtual Float evaluate(const Point2f &p) const {
        return windowedSinc(p.x, radius.x) * windowedSinc(p.y, radius.y);
    }
    
    Float sinc(Float x) const {
        x = std::abs(x);
        if (x < 1e-5) {
            return 1;
        }
        return std::sin(Pi * x) / (Pi * x);
    }
    
    Float windowedSinc(Float x, Float radius) const {
        x = std::abs(x);
        if (x > radius) {
        	return 0;
        }
        // todo可以优化保存τ的倒数
        Float lanczos = sinc(x / _tau);
        return sinc(x) * lanczos;
    }
    
    
private:
    const Float _tau;
};

USING_STD

shared_ptr<Serializable> createSincFilter(const nebJson &param);

PALADIN_END

#endif /* sinc_hpp */
