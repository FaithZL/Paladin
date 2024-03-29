//
//  filter.h
//  Paladin
//
//  Created by SATAN_Z on 2019/8/20.
//

#ifndef filter_h
#define filter_h

#include "core/header.h"
#include "core/cobject.h"
#include "tools/classfactory.hpp"
#include "math/sampling.hpp"

PALADIN_BEGIN

/**
 * 滤波器
 * 采样一个场景的数据，我们可以把场景数据当成一个函数，
 * 自变量为场景物体表面的点，函数值为该点的radiance，
 * 采样，必然会数据丢失，函数值为连续的，经过采样之后，我们获取到的函数只能以离散的形式保存数据
 * 因为我们只能保存样本点的函数值，没有采样的部分，函数值无法获取。
 *
 * 正式开始介绍滤波器
 * 我们要输出渲染的图像有如下步骤
 * 		1.通过图像样本重建出原始的连续图像L
 *   	2.预过滤函数L，移除像素间距超过奈奎斯特极限的任何频率
 *    	3.在像素的位置采样连续函数L，计算最终函数值，储存在像素中
 *    	因为没有必要显示的表达出函数L，所以我们可以用一个滤波器函数把第一步跟第二步合并
 *
 * 以一维函数为例，如果我们想通过离散的采样点完美重建原始函数，
 * 则采样率必须大于内奎斯特率，奈奎斯特采样率的定义为，函数最高频率的两倍
 * 这样我们可以用sinc滤波器来完美重建函数(奈奎斯特率的解释请移步本文件末尾)
 *
 * 但是在实际工作中，图像的最高频率都会超过我们能实现的采样频率，比如说物体的边缘
 * 因此，我们可以使用非均匀采样，用噪点(noise)来代替混叠(aliasing,也称走样)
 * 
 * 最近在采样理论领域的研究重新审视了重建问题，明确承认在实践中通常无法实现完美的重建。
 * 		特别是重构理论的研究目标已经从完善的重建转向了发展重建技术，
 *   	无论原始函数是否带限，都能将重建函数与原始函数之间的误差最小化。
 *
 * 图像在(x,y)像素上的值，表达式如下
 * I(x,y) = (∑f(x-xi,y-yi)w(xi,yi)L(xi,yi)) / (∑f(x-xi,y-yi))
 * 		L(xi,yi)为有效范围内的第i个样本的函数值
 * 		w(xi,yi)为相机显示的权重值
 * 		f为滤波函数，返回值也可以理解为权重
 * 
 */
const Point2f centerPos = Point2f(0.5,0.5);

class Filter : public CObject {
    
public:
    virtual ~Filter() {
        
    }
    
    Filter(const Vector2f &radius)
    : radius(radius),
    invRadius(Vector2f(1 / radius.x, 1 / radius.y)) {
        
    }
    
    virtual nloJson toJson() const {
        return nloJson();
    }
    
    /**
     * 也就是上述的f函数
     * @param  p (x-xi,y-yi)
     * @return   [description]
     */
    virtual Float evaluate(const Point2f &p) const = 0;
    
    void fillFilterTable();
    
    Point2f getPixelOffset(const Point2f &u, Float * weight) const;
    
    // 过滤半径，xy两个方向，超过半径的部分不处理
    const Vector2f radius;

    // 过滤半径的倒数
    const Vector2f invRadius;
    
    static CONSTEXPR int _filterTableWidth = 16;
    
    union {
        Float _filterTable[_filterTableWidth * _filterTableWidth];
        Float _filterTable2D[_filterTableWidth][_filterTableWidth];
    };
    
    
    Distribution2D _distribute2D;
    
    Distribution1D _distributeU;
    
    Distribution1D _distributeV;
};


PALADIN_END

/**
 * 想要了解奈奎斯特率
 * 这里需要顺带扯一下傅里叶分析方面的知识，伟大的物理学家傅里叶男爵认为
 * 		任何一个周期函数，都可以看做是若干个正弦波相叠加，相位不同，振幅不同，的正弦波的组合
 * 		周期函数分解成若干个正弦函数的过程称为，傅里叶级数展开
 *   	如果是非周期函数，可以看做是周期无限长的周期函数(前提是带限函数，频率有限)，
 *   	此时展开成正弦波的过程称为傅里叶变换
 *   	感觉自己对于傅里叶变换理解得不够深刻，todo，搞完主线再细啃一把
 *   	
 * ok，回到奈奎斯特率的解释，奈奎斯特率的定义就是函数中分解出的频率最高的正弦波的频率的两倍
 * 		换个角度来说，函数可以当成一个无限维的向量(函数横坐标是维度，纵坐标是该维度上分量的大小)，
 * 		一个周期函数可以分解出若干个分量，每个分量都是一个正弦波
 * 		奈奎斯特率就表示函数中频率最高的正弦波的频率的两倍，
 * 		其实很容易理解，就以一个正弦波为例，如果要完美重建该正弦波，采样率必须大于波的频率的两倍
 */

#endif /* filter_h */
