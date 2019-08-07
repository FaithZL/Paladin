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
Py(y) = Py(y(x)) = Px(x)
对 Py(y(x)) = Px(x) 两边求导得 

py(y) * dy/dx = px(x)  1式 一维分布之间的转换

dw = sinθdθdφ   2式  立体角公式不解释！

p(θ, φ)dθdφ = p(w)dw   3式

p(θ, φ) = sinθ p(w)  4式

如果X,Y两个随机变量相互独立，则
p(x, y) = px(x) * py(y)  5式

p(x,y)为2维随机变量的联合概率密度函数
则边缘概率密度函数，
p(x) = ∫p(x,y)dy    6式

条件概率密度函数
p(y|x) = p(x,y) / p(x)  7式

多维概率密度函数之间的转换

至于这个表达式如何推导，恕我直言，没搞过，等啃完主线任务再搞 todo
p1(y1,y2,....yn) = p1(T(x1,x2,....xn)) = p2(x1,x2,....xn) / |det(T)|

    | dT1/dx1 ... dT1/dxn |
    |    '  .       '     |
T = |    '      .   '     |
    |    '          '     |
    | dTn/dx1 ... dTn/dxn |

如何从p(x,y)空间转换到p(r,θ)空间？

1.  p1(x,y) = p1(T(r,θ)) = p2(r,θ) / |det(T)|
2.  x = rcosθ  y = rsinθ

    | dx/dr dx/dθ |     | cosθ -rsinθ |
T = |             |  =  |             | 
    | dy/dr dy/dθ |     | sinθ  rcosθ |

det(T) = rcosθcosθ + rsinθsinθ = r

可得
p(x,y) = p(θ,r)/r  8式

要生成指定分布的随机数，逆变换算法很常用
	积分求出指定pdf对应的cdf，记为F(x)
	y = F(x)定义域为[-∞,+∞],值域为[0,1]
	求出F(x)反函数G(x)
	G(x)定义域域为[0,1],值域为[-∞,+∞]
	在G(x)的定义域上均匀采样，得到的G(x)值就服从指定的pdf分布
	注释不方便画图，原理的话可以自行画图体验

 */
PALADIN_BEGIN

/*
 半球表面上生成均匀的随机点
 半球的立体角为2π
 ∫[0, 2π]p(w)dw = 1
 均匀分布p(w)为常数c，求解 c = p(w) = 1 / 2π，又由3式得 p(θ,φ) = sinθ / 2π

 p(θ) = ∫[0,2π]p(θ,φ)dφ = ∫[0,2π](sinθ/2π)dφ = sinθ  
 p(φ|θ) = p(φ,θ)/p(θ) = 1/2π
 只要能按照如上的pdf生成对应的θ与φ，就可以均匀采样半球了
 
 对p(θ)积分: P(θ) = ∫[0,θ]sinθ'dθ' = 1-cosθ
 对p(φ|θ)积分: P(φ|θ) = ∫[0,φ]1/2πdφ' = φ/2π
 有了上述两个累积分布函数，可以用逆变换算法求出θ与φ的指定分布
 a,b为[0,1]的均匀分布随机数
 θ = arccos(1 - a)，由于是均匀分布随机数1-a可以用a代替
 θ = arccos(a)
 φ = 2πb
 综合sphere.hpp中球的参数方程
 x = cos(2πb) * sqrt(1-a^2)
 y = sin(2πb) * sqrt(1-a^2)
 z = a
 */
Vector3f uniformSampleHemisphere(const Point2f &u);

/*
 在一个球表面上生成均匀的随机点
 球的立体角为4π
 ∫[0, 4π]p(w)dw = 1
 均匀分布p(w)为常数c，求解 c = p(w) = 1 / 4π，又由3式得 p(θ,φ) = sinθ/4π

 p(θ) = ∫[0,2π]p(θ,φ)dφ = ∫[0,2π](sinθ/4π)dφ = sinθ/2
 p(φ|θ) = p(φ,θ)/p(θ) = 1/2π

 对p(θ)积分: P(θ) = ∫[0,θ](1/2)sinθ'dθ' = (1/2)*(1-cosθ)
 对p(φ|θ)积分: P(φ|θ) = ∫[0,φ]1/2πdφ' = φ/2π
 a,b为[0,1]的均匀分布随机数
 θ = arccos(1-2a)
 φ = 2πb
 综合sphere.hpp中球的参数方程
 x = cos(2πb) * 2 * sqrt(a-a^2)
 y = sin(2πb) * 2 * sqrt(a-a^2)
 z = 1 - 2a 
 */
Vector3f uniformSampleSphere(const Point2f &u);


/*
 在一个局部球表面上生成均匀的随机点
 三个参数控制局部球，θmin θmax φmax
 ∫p(w)dw = 1
 球的立体角为dw = sinθdθdφ
 局部球面面积为 s = ∫[0, φmax]∫[θmin, θmax]sinθdθdφ = φmax(cosθmin - cosθmax)

 均匀分布p(w)为常数c，求解 c = p(w) = 1 / s，又由3式得 p(θ,φ) = sinθ/s

 p(θ) = ∫[0, φmax]p(θ,φ)dφ = ∫[0, φmax](sinθ/s)dφ = sinθ/(cosθmin - cosθmax)
 p(φ|θ) = p(φ,θ)/p(θ) = 1/φmax

 对p(θ)积分: P(θ) = ∫[0,θ]sinθ'/(cosθmin - cosθmax)dθ' = (cosθmin - cosθ)/(cosθmin - cosθmax)
 对p(φ|θ)积分: P(φ|θ) = ∫[0,φ]1/φmaxdφ' = φ/φmax
 a,b为[0,1]的均匀分布随机数
 θ = arccos(cosθmin - a(cosθmin - cosθmax))
 φ = φmax * b

 sinθ = sqrt(1 - (cosθ)^2)
 求得
 x = sinθcosφ = sinθcos(b * φmax)
 y = sinθsinφ = sinθsin(b * φmax)
 z = cosθ = cosθmin - a(cosθmin - cosθmax)

 */
Vector3f uniformSamplePartialSphere(const Point2f &u, Float phiMax, Float cosThetaMin, Float cosThetaMax);

/*
 均匀采样一个圆锥的pdf函数

 p(θ, φ) = sinθ p(w)
 p(θ, φ) = pθ(θ) * pφ(φ)
 pφ(φ) = 1/2π

 p(w) = pθ(θ)/(2π * sinθ) 为常数
 所以p(θ)/sinθ为常数
 假设p(θ) = c * sinθ

 1 = ∫[0,θmax]p(θ)dθ
 求得 c = 1/(1 - cosθmax)
 p(θ) = sinθ/(1 - cosθmax)
 p(w) = p(θ, φ)/sinθ = pθ(θ) * pφ(φ)/sinθ = 1/(2π(1-cosθmax))
 */
Float uniformConePdf(Float cosThetaMax);

/*
 均匀采样一个圆锥，默认圆锥的中心轴为(0,0,1)，圆锥顶点为坐标原点
 可以认为圆锥采样是sphere，hemisphere采样的一般化
 当圆锥采样的θmax为π/2时，圆锥采样为hemisphere采样
 当圆锥采样的θmax为π时，圆锥采样为sphere采样

 p(θ) = sinθ/(1 - cosθmax)

 积分计算得到累积分布函数
 P(θ) = (cosθ - 1)/(cosθmax - 1)
 P(φ) = φ/2π
 
 a,b为[0,1]的均匀分布随机数
 cosθ = (1 - a) + a * cosθmax
 φ = 2πb

 sinθ = sqrt(1 - cosθ * cosθ)

 x = sinθcosφ
 y = sinθsinφ
 z = cosθ
 */
Vector3f uniformSampleCone(const Point2f &u, Float cosThetaMax);

/*
 均匀分布可得p(x,y) = 1/π
 p(x,y) = p(θ,r)/r 8式
 又由8式，可得
 p(θ,r) = r/π
 由边缘概率密度函数公式可得
 p(r) = ∫[0,2π]p(θ,r)dθ = 2r
 p(θ|r) = p(θ,r)/p(r) = 1/2π
 θ与r相互独立 p(θ|r) = 1/2π = p(θ)
 对p(θ)积分可得
 P(θ) = θ/2π
 对p(r)积分可得
 P(r) = r^2

 a,b为[0,1]的均匀分布随机数
 r = √(a)
 θ = 2πb

*/
Point2f uniformSampleDisk(const Point2f &u);


/*
 均匀采样扇形，其中扇形角度为 θmax
 面积为 s = θmax/2
 
 均匀分布可得p(x,y) = 1/s
 p(x,y) = p(θ,r)/r 8式
 又由8式，可得
 p(θ,r) = 2r/θmax
 由边缘概率密度函数公式可得
 p(r) = ∫[0,θmax]p(θ,r)dθ = 2r
 p(θ|r) = p(θ,r)/p(r) = 1/θmax
 θ与r相互独立 p(θ|r) = 1/θmax = p(θ)
 对p(θ)积分可得
 P(θ) = θ/θmax
 对p(r)积分可得
 P(r) = r^2

 a,b为[0,1]的均匀分布随机数
 r = √(a)
 θ = θmaxb

*/
Point2f uniformSampleSector(const Point2f &u, Float thetaMax);


/*
 均匀采样局部扇形，其中扇形角度为 θmax，外径为1，内径为rMin
 0 <= rMin <= 1
 面积为 s = θmax/2 * (1 - rMin^2)
 
 均匀分布可得p(x,y) = 1/s
 p(x,y) = p(θ,r)/r 8式
 又由8式，可得
 p(θ,r) = 2r/(θmax(1 - rMin^2))
 由边缘概率密度函数公式可得
 p(r) = ∫[0,θmax]p(θ,r)dθ = 2r/(1 - rMin^2)
 p(θ|r) = p(θ,r)/p(r) = 1/θmax
 θ与r相互独立 p(θ|r) = 1/θmax = p(θ)
 对p(θ)积分可得
 P(θ) = θ/θmax
 对p(r)积分可得
 P(r) = (r^2 - rMin^2)/(1 - rMin^2)

 a,b为[0,1]的均匀分布随机数
 r = √(a(1 - rMin^2) + rMin^2)
 θ = θmaxb

*/
Point2f uniformSamplePartialSector(const Point2f &u, Float thetaMax, Float rMin);

/*
 均匀采样三角形
 转换为均匀采样直角三角形，直角边分别为uv，长度为1
 三角形面积为s = 1/2
 p(u, v) = 2
 p(u) = ∫[0, 1-u]p(u, v)dv = 2(1 - u)
 p(u|v) = p(u,v)/p(u) = 1/(1 - u)
 积分得
 P(u) = ∫[0, u]p(u')du' = 2u - u^2
 P(v) = ∫[0, v]p(u|v')dv' = v/(1 - u)
 
 ab为均匀分布的随机数
 对P(u) P(v)求反函数，得
 u = 1 - √a
 v = b * √a
 */
Point2f uniformSampleTriangle(const Point2f &u);

PALADIN_END


#endif /* sampling_hpp */





