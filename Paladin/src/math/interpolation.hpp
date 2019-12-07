//
//  interpolation.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/12/6.
//

#ifndef interpolation_hpp
#define interpolation_hpp

#include "core/header.h"

PALADIN_BEGIN

/**
 * 以下介绍的是插值方式，样条插值(Spline Interpolation)
 * Catmull–Rom样条插值，我们之前使用过的插值，线性插值比较多，线性插值还是比较简单
 * 
 * 所谓插值，就是给定两个点的，让计算机脑补两点之间的任意一点，脑补的方式很多
 * 下面介绍一下Catmull–Rom样条插值
 * 给定一系列的x0,x1,x2,x3,.....xn，对每个子区间进行插值
 * 
 * 针对一个子区间[x_i, x_i+1]，插值算出区间内任意一点，我们用一个三次多项式去近似插值
 *
 *           f(x) = ax^3 + bx^2 + cx + d
 *
 * 每个子区间都用f(x)进行插值，都有对应的abcd四个系数，
 *
 *         p_i(x_i) = f(x_i)
 *         p_i+1(x_i+1) = f(x_i+1)
 *
 * 为了简单的描述，我们把目标放在[x0,x1]区间假设区间为，[0,1],将x0,x1带入函数之后，得
 *
 *           f(x0) = a(x0)^3 + b(x0)^2 + c(x0) + d
 *           f(x1) = a(x1)^3 + b(x1)^2 + c(x1) + d
 *           f'(x0) = 3a(x0)^2 + 2b(x0) + c
 *           f'(x1) = 3a(x1)^2 + 2b(x1) + c
 *
 * f(x0),f(x1)是已知数，由于区间比较小，
 * f'(x0)可以用p_1,p_-1两点连线的斜率近似
 * f'(x1)可以用p_0,p_2两点连线的斜率近似
 *           
 *           f(x2) - f(x0)
 * f'(x1) ≈ ---------------               1式
 *              x2 - x0
 *
 *           f(x1) - f(x_-1)
 * f'(x0) = -----------------             2式
 *              x1 - x_-1
 * 
 * 以上有四个未知数，四个方程式，妥妥的解了
 *           a = f'(x0) + f'(x1) + 2f(x0) - 2f(x1)
 *           b = 3f(x1) - 3f(x0) - 2f'(x0) - f(x1)
 *           c = f'(x0)
 *           d = f(x0)
 *
 * 带入之后，整理得
 * 			p(x) = (2x^3 − 3x^2 + 1)f(x0)        3式
 *          	+ (−2x^3 + 3x^2)f(x1)
 *          	+ (x^3 − 2x^2 + x)f'(x0) 
 *          	+ (x^3 − x^2)f'(x1)
 *
 * 将1，2式带入3式之后
 *                   x^3 - 2x^2 + x
 *          p(x) = ------------------ f(x_-1)
 *                     x1 - x_-1
 *
 *                                     x^3 - x^2
 *               + (2x^3 - 3x^2 + 1 - -----------) f(x0)
 *                                        x^2
 *               
 *                                  x^3 - 2x^2 + x
 *               + (-2x^3 + 3x^2 + ----------------) f(x1)
 *                                    x1 - x_-1
 *                                    
 *                   x^3 - x^2
 *               + ------------- f(x2)
 *                    x2 - x0
 *
 * 于是，我们便得到了插值方程。
 * 
 * 我们用 w0,w1,w2,w3表示 f(x_-1)，f(x0)，f(x1)，f(x2)的权重
 *
 * 
 */
Float CatmullRom(int size, const Float *nodes, const Float *values, Float x);

bool CatmullRomWeights(int size, const Float *nodes, Float x, int *offset,
                       Float *weights);

Float SampleCatmullRom(int size, const Float *nodes, const Float *f,
                       const Float *cdf, Float sample, Float *fval = nullptr,
                       Float *pdf = nullptr);

Float SampleCatmullRom2D(int size1, int size2, const Float *nodes1,
                         const Float *nodes2, const Float *values,
                         const Float *cdf, Float alpha, Float sample,
                         Float *fval = nullptr, Float *pdf = nullptr);

Float IntegrateCatmullRom(int n, const Float *nodes, const Float *values,
                          Float *cdf);

Float InvertCatmullRom(int n, const Float *x, const Float *values, Float u);


PALADIN_END

#endif /* interpolation_hpp */
