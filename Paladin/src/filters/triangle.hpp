//
//  triangle.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/9/13.
//

#ifndef filters_triangle_hpp
#define filters_triangle_hpp

#include "core/filter.h"

PALADIN_BEGIN

/**
 * 三角滤波器，比盒式滤波器效果略好
 * 函数值随着像素中心线性衰减
 */
class TriangleFilter : public Filter {
    
public:
    
    TriangleFilter(const Vector2f &radius) : Filter(radius) {
        fillFilterTable();
    }
    
    virtual Float evaluate(const Point2f &p) const override {
        return std::max((Float)0, radius.x - std::abs(p.x)) *
                std::max((Float)0, radius.y - std::abs(p.y));
    }
    
};

USING_STD

CObject_ptr createTriangleFilter(const nloJson &);

PALADIN_END

#endif /* filters_triangle_hpp */

