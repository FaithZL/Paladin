//
//  sampling.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef sampling_hpp
#define sampling_hpp

#include "core/header.h"
#include "math/rng.h"
/*
P为累积分布函数(cdf)，p为概率密度函数(pdf)，cdf求导得到pdf
常用概率论基础公式

Pr{X ≤ x} = Pr{Y ≤ y(x)}
Py(y) = Py(y(x)) = Px(x)
对 Py(y(x)) = Px(x) 两边求导得 

py(y) * dy/dx = px(x)  1式 一维分布之间的转换

dw = sinθdθdφ   2式  立体角公式不解释！

p(θ, φ)dθdφ = p(w)dw   3式 (p(θ, φ)与p(w)分别在定义域上积分，都为1)

p(θ, φ) = sinθ p(w)  4式   (2，3两式，联合)

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

/**
 * 半球表面上生成均匀的随机点
 * 半球的立体角为2π
 * ∫[0, 2π]p(w)dw = 1
 * 均匀分布p(w)为常数c，求解 c = p(w) = 1 / 2π，又由3式得 p(θ,φ) = sinθ / 2π
 *
 * p(θ) = ∫[0,2π]p(θ,φ)dφ = ∫[0,2π](sinθ/2π)dφ = sinθ  
 * p(φ|θ) = p(φ,θ)/p(θ) = 1/2π
 * 只要能按照如上的pdf生成对应的θ与φ，就可以均匀采样半球了
 *
 * 对p(θ)积分: P(θ) = ∫[0,θ]sinθ'dθ' = 1-cosθ
 * 对p(φ|θ)积分: P(φ|θ) = ∫[0,φ]1/2πdφ' = φ/2π
 * 有了上述两个累积分布函数，可以用逆变换算法求出θ与φ的指定分布
 * a,b为[0,1]的均匀分布随机数
 * θ = arccos(1 - a)，由于是均匀分布随机数1-a可以用a代替
 * θ = arccos(a)
 * φ = 2πb
 * 综合sphere.hpp中球的参数方程
 * x = cos(2πb) * sqrt(1-a^2)
 * y = sin(2πb) * sqrt(1-a^2)
 * z = a
 */
Vector3f uniformSampleHemisphere(const Point2f &u);

/**
 * 在一个球表面上生成均匀的随机点
 * 球的立体角为4π
 * ∫[0, 4π]p(w)dw = 1
 * 均匀分布p(w)为常数c，求解 c = p(w) = 1 / 4π，又由3式得 p(θ,φ) = sinθ/4π
 *
 * p(θ) = ∫[0,2π]p(θ,φ)dφ = ∫[0,2π](sinθ/4π)dφ = sinθ/2
 * p(φ|θ) = p(φ,θ)/p(θ) = 1/2π
 *
 * 对p(θ)积分: P(θ) = ∫[0,θ](1/2)sinθ'dθ' = (1/2)*(1-cosθ)
 * 对p(φ|θ)积分: P(φ|θ) = ∫[0,φ]1/2πdφ' = φ/2π
 * a,b为[0,1]的均匀分布随机数
 * θ = arccos(1-2a)
 * φ = 2πb
 * 综合sphere.hpp中球的参数方程
 * x = cos(2πb) * 2 * sqrt(a-a^2)
 * y = sin(2πb) * 2 * sqrt(a-a^2)
 * z = 1 - 2a 
 */
Vector3f uniformSampleSphere(const Point2f &u);


/**
 * 在一个局部球表面上生成均匀的随机点
 * 三个参数控制局部球，θmin θmax φmax
 * ∫p(w)dw = 1
 * 球的立体角为dw = sinθdθdφ
 * 局部球面面积为 s = ∫[0, φmax]∫[θmin, θmax]sinθdθdφ = φmax(cosθmin - cosθmax)
 * 
 * 均匀分布p(w)为常数c，求解 c = p(w) = 1 / s，又由3式得 p(θ,φ) = sinθ/s
 * 
 * p(θ) = ∫[0, φmax]p(θ,φ)dφ = ∫[0, φmax](sinθ/s)dφ = sinθ/(cosθmin - cosθmax)
 * p(φ|θ) = p(φ,θ)/p(θ) = 1/φmax
 * 
 * 对p(θ)积分: P(θ) = ∫[0,θ]sinθ'/(cosθmin - cosθmax)dθ' = (cosθmin - cosθ)/(cosθmin - cosθmax)
 * 对p(φ|θ)积分: P(φ|θ) = ∫[0,φ]1/φmaxdφ' = φ/φmax
 * a,b为[0,1]的均匀分布随机数
 * θ = arccos(cosθmin - a(cosθmin - cosθmax))
 * φ = φmax * b
 * 
 * sinθ = sqrt(1 - (cosθ)^2)
 * 求得
 * x = sinθcosφ = sinθcos(b * φmax)
 * y = sinθsinφ = sinθsin(b * φmax)
 * z = cosθ = cosθmin - a(cosθmin - cosθmax)
 *    
 * @param  u           [均匀随机变量]
 * @param  phiMax      
 * @param  cosThetaMin [description]
 * @param  cosThetaMax [description]
 * @return             单位向量
 */
Vector3f uniformSamplePartialSphere(const Point2f &u, Float phiMax, Float cosThetaMin, Float cosThetaMax);

/**
 * 均匀采样一个圆锥的pdf函数
 * 
 * p(θ, φ) = sinθ p(w)
 * p(θ, φ) = pθ(θ) * pφ(φ)
 * pφ(φ) = 1/2π
 * 
 * p(w) = pθ(θ)/(2π * sinθ) 为常数
 * 所以p(θ)/sinθ为常数
 * 假设p(θ) = c * sinθ
 * 
 * 1 = ∫[0,θmax]p(θ)dθ
 * 求得 c = 1/(1 - cosθmax)
 * p(θ) = sinθ/(1 - cosθmax)
 * p(w) = p(θ, φ)/sinθ = pθ(θ) * pφ(φ)/sinθ = 1/(2π(1-cosθmax))
 *
 */
Float uniformConePdf(Float cosThetaMax);

/**
 * 均匀采样一个圆锥，默认圆锥的中心轴为(0,0,1)，圆锥顶点为坐标原点
 * 可以认为圆锥采样是sphere，hemisphere采样的一般化
 * 当圆锥采样的θmax为π/2时，圆锥采样为hemisphere采样
 * 当圆锥采样的θmax为π时，圆锥采样为sphere采样
 *
 * p(θ) = sinθ/(1 - cosθmax)
 *
 * 积分计算得到累积分布函数
 * P(θ) = (cosθ - 1)/(cosθmax - 1)
 * P(φ) = φ/2π
 * 
 * a,b为[0,1]的均匀分布随机数
 * cosθ = (1 - a) + a * cosθmax
 * φ = 2πb
 *
 * sinθ = sqrt(1 - cosθ * cosθ)
 *
 * x = sinθcosφ
 * y = sinθsinφ
 * z = cosθ
 */
Vector3f uniformSampleCone(const Point2f &u, Float cosThetaMax);

/**
 * 均匀分布可得p(x,y) = 1/π
 * p(x,y) = p(θ,r)/r 8式
 * 又由8式，可得
 * p(θ,r) = r/π
 * 由边缘概率密度函数公式可得
 * p(r) = ∫[0,2π]p(θ,r)dθ = 2r
 * p(θ|r) = p(θ,r)/p(r) = 1/2π
 * θ与r相互独立 p(θ|r) = 1/2π = p(θ)
 * 对p(θ)积分可得
 * P(θ) = θ/2π
 * 对p(r)积分可得
 * P(r) = r^2
 *
 * a,b为[0,1]的均匀分布随机数
 * r = √(a)
 * θ = 2πb
 */
Point2f uniformSampleDisk(const Point2f &u);


/**
 * 先把圆盘平均分为8个扇形，每个扇形的圆心角为45°
 * 我们针对每个扇形进行采样
 * 一个扇形内部的点可以理解为一个等腰直角三角形内部点的映射
 * (x,y)到 (r,θ)的映射为
 * r = x
 * θ = y/x * π/4
 * 整个映射方式相当于把一个边长为2的正方形的采样点通过上述方式
 * 映射到一个直径为2的圆盘
 * 知道以上思路之后，代码就很简单了
 * 至于为何这个方式比uniformSampleDisk要常用，目前还没有搞懂，
 * 
 * 用python生成了一波数据与uniformSampleDisk对比
 * 序列采用[0,1]的2维均匀点阵，concentricSampleDisk的均匀程度明显
 * 优于uniformSampleDisk采样
 * 但随机数序列生成的样本差别不明显
 * 用python实现了同样的算法，测试耗时uniformSampleDisk要少一些，
 * 所以暂时打算使用uniformSampleDisk作为圆盘采样函数
 * 
 * 其实这点很容易理解，如果用一个100 * 100均匀点阵采样一个长宽比为10:1的矩形
 * 显然，效果不会很好
 */
Point2f concentricSampleDisk(const Point2f &u);


/**
 * 均匀采样扇形，其中扇形角度为 θmax
 * 面积为 s = θmax/2
 *
 * 均匀分布可得p(x,y) = 1/s
 * p(x,y) = p(θ,r)/r 8式
 * 又由8式，可得
 * p(θ,r) = 2r/θmax
 * 由边缘概率密度函数公式可得
 * p(r) = ∫[0,θmax]p(θ,r)dθ = 2r
 * p(θ|r) = p(θ,r)/p(r) = 1/θmax
 * θ与r相互独立 p(θ|r) = 1/θmax = p(θ)
 * 对p(θ)积分可得
 * P(θ) = θ/θmax
 * 对p(r)积分可得
 * P(r) = r^2
 *
 * a,b为[0,1]的均匀分布随机数
 * r = √(a)
 * θ = θmax * b
 */
Point2f uniformSampleSector(const Point2f &u, Float thetaMax);


/**
 * 均匀采样局部扇形，其中扇形角度为 θmax，外径为1，内径为rMin
 * 0 <= rMin <= 1
 * 面积为 s = θmax/2 * (1 - rMin^2)
 *
 * 均匀分布可得p(x,y) = 1/s
 * p(x,y) = p(θ,r)/r 8式
 * 又由8式，可得
 * p(θ,r) = 2r/(θmax(1 - rMin^2))
 * 由边缘概率密度函数公式可得
 * p(r) = ∫[0,θmax]p(θ,r)dθ = 2r/(1 - rMin^2)
 * p(θ|r) = p(θ,r)/p(r) = 1/θmax
 * θ与r相互独立 p(θ|r) = 1/θmax = p(θ)
 * 对p(θ)积分可得
 * P(θ) = θ/θmax
 * 对p(r)积分可得
 * P(r) = (r^2 - rMin^2)/(1 - rMin^2)
 *
 * a,b为[0,1]的均匀分布随机数
 * r = √(a(1 - rMin^2) + rMin^2)
 * θ = θmaxb
 */
Point2f uniformSamplePartialSector(const Point2f &u, Float thetaMax, Float rMin);

/**
 *  均匀采样三角形
 * 转换为均匀采样直角三角形，直角边分别为uv，长度为1
 * 三角形面积为s = 1/2
 * p(u, v) = 2
 * p(u) = ∫[0, 1-u]p(u, v)dv = 2(1 - u)
 * p(u|v) = p(u,v)/p(u) = 1/(1 - u)
 * 积分得
 * P(u) = ∫[0, u]p(u')du' = 2u - u^2
 * P(v) = ∫[0, v]p(u|v')dv' = v/(1 - u)
 *
 * ab为均匀分布的随机数
 * 对P(u) P(v)求反函数，得
 * u = 1 - √a
 * v = b * √a
 * @param  u 均匀二维随机变量
 * @return   三角形内部uv坐标
 */
Point2f uniformSampleTriangle(const Point2f &u);



/**
 * 随机打乱一个数组
 * 流程大体如下
 *
 *     0     1     2     3     4     5   count
 * 0   s0    s3    s6    s9    s12   s15
 * 1   s1    s4    s7    s10   s13   s16
 * 2   s2    s5    s8    s11   s14   s17
 * dim
 * 
 *   i = 0, other = randomInt(5 - 0) + 0 = 2
 *
 *   j=0, swap(s[3*0 + 0], s[3*2 + 0]) swap(s[0], s[6])
 *   j=1, swap(s[3*0 + 1], s[3*2 + 1]) swap(s[1], s[7])
 *   j=0, swap(s[3*0 + 2], s[3*2 + 2]) swap(s[2], s[8])
 *   ......
 *   i = 1, other = randomInt(5 - 1) + 1 = 3
 *
 *   j=0, swap(s[3*1 + 0], s[3*3 + 0]) swap(s[3], s[9])
 *   j=1, swap(s[3*1 + 1], s[3*3 + 1]) swap(s[4], s[10])
 *   j=0, swap(s[3*1 + 2], s[3*3 + 2]) swap(s[5], s[11])
 *   ......
 *
 *   i = 2, other = randomInt(5 - 2) + 2 = 3
 *
 *   j=0, swap(s[3*2 + 0], s[3*3 + 0]) swap(s[6], s[9])
 *   j=1, swap(s[3*2 + 1], s[3*3 + 1]) swap(s[7], s[10])
 *   j=0, swap(s[3*2 + 2], s[3*3 + 2]) swap(s[8], s[11])
 *   ......
 * 上述过程可以看做有count个dim维的向量，进行重排，对于向量是整体重排  
 * 
 * @param samp        样本列表首地址 
 * @param count       样本个数
 * @param nDimensions 样本向量的维度
 * @param rng         随机数生成器
 */
template <typename T> 
void shuffle(T *samp, int count, int nDimensions, RNG &rng) {
    for (int i = 0; i < count; ++i) {
        // 随机选择一个i右侧的索引
        int other = i + rng.uniformUInt32(count - i);
        for (int j = 0; j < nDimensions; ++j) {
            std::swap(samp[nDimensions * i + j], samp[nDimensions * other + j]);
        }
    }
}

/**
 * 一维分层采样
 * 代码中比较明显了，其实就是给一维均匀分布添加了一个扰动
 * 使得不序列不那么均匀
 * @param samples  样本数组指针
 * @param nsamples 样本个数
 * @param rng      随机生成器
 * @param jitter   是否扰动
 */
void stratifiedSample1D(Float *samples, int nsamples, RNG &rng, bool jitter = true);

/**
 * 二维分层采样
 * 原理同上
 * @param samples 样本数组指针
 * @param nx      x方向上样本个数
 * @param ny      y方向上样本个数
 * @param rng     随机数生成器
 * @param jitter  是否扰动
 */
void stratifiedSample2D(Point2f *samples, int nx, int ny, RNG &rng, bool jitter = true);

/**
 * 基本思路：
 * 可以理解为有个nDim维向量，每个维度采样nSamples个样本
 * 之后对每个维度的样本进行随机重排列
 * 注意重排的方式与shuffle不同，shuffle是样本点直接的重排列
 * 而拉丁超立方体采样的重排列是针对每个维度分量的重排
 * @param samples  样本列表首地址
 * @param nSamples 样本数量
 * @param nDim     维数
 * @param rng      随机数生成器
 */
void latinHypercube(Float *samples, int nSamples, int nDim, RNG &rng);


/**
 * cosine采样半球表面
 * 思路非常简单
 * 先均匀采样圆盘，然后通过圆盘映射到半球表面
 * @param  u 均匀样本点
 */
inline Vector3f cosineSampleHemisphere(const Point2f &u) {
    Point2f d = uniformSampleDisk(u);
    Float z = std::sqrt(std::max((Float)0, 1 - d.x * d.x - d.y * d.y));
    return Vector3f(d.x, d.y, z);
}

/**
 * 单位半球的表面积2π
 * 概率密度函数值为1/2π
 */
inline Float uniformHemispherePdf() {
    return Inv2Pi;
}

/**
 * 单位球体的表面积为4π
 * 概率密度函数值为1/4π
 */
inline Float uniformSpherePdf() {
    return Inv4Pi;
}

inline Float balanceHeuristic(int nf, Float fPdf, int ng, Float gPdf) {
    return (nf * fPdf) / (nf * fPdf + ng * gPdf);
}

inline Float powerHeuristic(int nf, Float fPdf, int ng, Float gPdf) {
    Float f = nf * fPdf, g = ng * gPdf;
    return (f * f) / (f * f + g * g);
}

/**
 * 一维分布结构
 * 用分段直线函数取逼近曲线
 * 如果一个分布没有明确的表达式，则可以使用该类
 * 由一个定义域为[0,1]的分段函数构造而来
 * 理论上_func函数应该恒大于零
 */
struct Distribution1D {
    Distribution1D(const Float *f, int num)
    :_func(f, f + num),
    _cdf(num + 1) {
        _cdf[0] = 0;
        for (int i = 1; i < num + 1; ++i) {
            // todo 这里可以优化
            _cdf[i] = _cdf[i - 1] + _func[i - 1] / num;
        }

        // 由于func积分值不一定为1
        // 所以需要将_cdf归一化
        _funcInt = _cdf[num];
        if (_funcInt == 0) {
            // 如果_func全为零，则均匀分布
            for (int i = 1; i < num + 1; ++i) {
                // todo 这里可以优化
                _cdf[i] = Float(i) / Float(num);
            }
        } else {
            for (int i = 1; i < num + 1; ++i) {
                // todo 这里可以优化
                _cdf[i] = _cdf[i] / _funcInt;
            }
        }
    }

    int count() const {
        return (int)_func.size();
    }

    /**
     * 根据均匀随机变量u
     * 生成符合func分布的随机变量
     * 用逆变换算法，过程就不赘述了
     * @param  u   随机变量
     * @param  pdf 返回的概率密度函数值
     * @param  off x轴偏移量
     * @return     返回[0,1]之间的浮点数
     */
    Float sampleContinuous(Float u, Float *pdf = nullptr, int *off = nullptr) const {
        auto predicate = [&](int index) { 
            return _cdf[index] <= u; 
        };
        int offset = findInterval((int)_cdf.size(), predicate);
        if (off) {
            *off = offset;
        }
        Float du = u - _cdf[offset];
        if ((_cdf[offset + 1] - _cdf[offset]) > 0) {
            CHECK_GT(_cdf[offset + 1], _cdf[offset]);
            du /= (_cdf[offset + 1] - _cdf[offset]);
        }
        DCHECK(!std::isnan(du));

        if (pdf) {
            *pdf = (_funcInt > 0) ? _func[offset] / _funcInt : 0;
        }
        return (offset + du) / count();        
    }

    /**
     * 离散采样
     * @param  u         随机变量
     * @param  pdf       对应的概率密度函数值
     * @param  uRemapped 重新映射的u变量
     * @return           返回偏移量
     */
    int sampleDiscrete(Float u, Float *pdf = nullptr, Float *uRemapped = nullptr) const {
        auto predicate = [&](int index) { 
            return _cdf[index] <= u; 
        };
        int offset = findInterval((int)_cdf.size(), predicate);
        if (pdf) {
            // 保证pdf积分为1，所以比连续形式的pdf多除了一个count
            *pdf = (_funcInt > 0) ? _func[offset] / (_funcInt * count()) : 0;
        }
        if (uRemapped) {
            *uRemapped = (u - _cdf[offset]) / (_cdf[offset + 1] - _cdf[offset]);
        }
        if (uRemapped) {
            DCHECK(*uRemapped >= 0.f && *uRemapped <= 1.f);
        }
        return offset;        
    }

    /**
     * 获取离散形式的pdf
     * @param  index 索引
     * @return       pdf
     */
    Float discretePDF(int index) const {
        DCHECK(index >= 0 && index < count());
        return _func[index] / (_funcInt * count());
    }

private:
    // 指定分布的函数
    std::vector<Float> _func;
    // 指定函数的累积分布函数
    std::vector<Float> _cdf;
    // func函数的积分值
    Float _funcInt;

    friend struct Distribution2D;
};

/**
 * 2维分布
 * 分为uv两个维度
 * 个人觉得这个知识点比较简单，就不细说了
 *
 */
struct Distribution2D {
public:
    Distribution2D(const Float *data, int nu, int nv) {
        _pConditionalV.reserve(nv);
        // 创建v个长度为u的一维分布，存入列表中
        for (int v = 0; v < nv; ++v) {
            _pConditionalV.emplace_back(new Distribution1D(&data[v * nu], nu));
        }
        std::vector<Float> _marginalFunc;
        _marginalFunc.reserve(nv);
        // 将每个一维分布的积分值存入_pMarginal中作为边缘概率密度
        for (int v = 0; v < nv; ++v) {
            _marginalFunc.push_back(_pConditionalV[v]->_funcInt);
        }
        _pMarginal.reset(new Distribution1D(&_marginalFunc[0], nv));
    }

    Point2f sampleContinuous(const Point2f &u, Float *pdf) const {
        Float pdfs[2];
        int v;
        Float d1 = _pMarginal->sampleContinuous(u[1], &pdfs[1], &v);
        Float d0 = _pConditionalV[v]->sampleContinuous(u[0], &pdfs[0]);
        *pdf = pdfs[0] * pdfs[1];
        return Point2f(d0, d1);
    }

    Float pdf(const Point2f &p) const {
        int iu = clamp(int(p[0] * _pConditionalV[0]->count()), 0, _pConditionalV[0]->count() - 1);
        int iv = clamp(int(p[1] * _pMarginal->count()), 0, _pMarginal->count() - 1);
        return _pConditionalV[iv]->_func[iu] / _pMarginal->_funcInt;
    }

private:
    // 长度为v的列表，每个列表储存一个样本数量为u的一维分布对象
    std::vector<std::unique_ptr<Distribution1D>> _pConditionalV;
    // u趋向于1时，v的边缘概率密度函数
    std::unique_ptr<Distribution1D> _pMarginal;
};

PALADIN_END


#endif /* sampling_hpp */





