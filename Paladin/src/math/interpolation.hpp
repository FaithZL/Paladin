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
 * 为了简单的描述，我们把目标放在[x0,x1]区间，假设区间为[0,1],将x0,x1带入函数之后，得
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
 *                                      x2 - x0
 *               
 *                                  x^3 - 2x^2 + x
 *               + (-2x^3 + 3x^2 - ----------------) f(x1)
 *                                    x1 - x_-1
 *                                    
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

/**
 * 随机采样CatmullRom曲线
 *
 * 累积分布函数形式如下
 * 
 *       0                                 i = 0
 * Fi =  
 *      i-1               1
 *       ∑  ∫[xk, x_k+1] --- fk(x') dx'    i > 0
 *      k=0               c
 *
 * 其中，c为归一化参数
 *
 * c = ∫[x0, x_n-1] f(x) dx
 *
 * 一个直接的方法是找到均匀随机变量u的所在cdf的区间
 * 
 *         Fi <= ξ1 <= Fi+1
 *         
 * 由于F函数单调递增，则用如下方式可以求出i
 *
 *         Fi <= ξ1 * Fn-1 <= Fi+1
 *
 *
 *         ξ1 * Fn−1 − Fi 
 * ξ2 = --------------------   4式
 *           Fi+1 − Fi
 *
 * ξ2为子区间内的随机变量值，用于采样[xi,x_i+1]区间的累积分布函数
 *
 *        Fi(x) = ∫[xi,x] f(x')dx'
 *
 *
 * x = F^-1(Fi(x_i+1) * ξ2)   5式
 * 其中将4式带入5式，又因为Fi(x_i+1) = F_i+1 - Fi
 *
 * x = F^-1(ξ1 * Fn−1 − Fi)
 * 
 */
Float SampleCatmullRom(int size, const Float *nodes, const Float *f,
                       const Float *cdf, Float u, Float *fval = nullptr,
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
