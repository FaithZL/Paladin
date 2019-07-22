//
//  errfloat.h
//  Paladin
//
//  Created by SATAN_Z on 2019/7/21.
//  Copyright © 2019 Zero. All rights reser_valed.
//  

#ifndef errfloat_h
#define errfloat_h

#include "header.h"

PALADIN_BEGIN

/*
由于浮点误差产生的错误一般会出现在ray与shape相交的计算中，由于浮点误差，计算出的交点
往往出现在shape的内部或者shape的外部，而不是正好在shape边缘上，如果是ray与shape的交点处的切线夹角很小
那么误差产生的错位会更加严重。为了解决这种误差，有两种方案
	1.简单粗暴，用双精度浮点类型，double
	这种方案的好处在于简单，不需要做额外任何处理，坏处显而易见，double类型占8个字节，float只占4个字节
	首先内存占用大，可能导致cache miss的概率就高，内存对齐不灵活
	其次，经过simd优化之后float的运算效率比double高一倍

所以在离线渲染中用到的降低误差的办法不是简单的使用double，而是接下来要介绍的一种方式
先简单介绍一下float的内存结构

float占4个字节，总共32位，从左边的高位开始排列，最高的一位是符号位，之后8位为指数位，最低的23为有效数字位
任何一个浮点数都可以写成科学计数法的形式，比如说
253 = 2.53 * 10^2，同样可以表示二进制的方式 a * 2 ^ b，a储存在的23个有效数字位中，b储存在8个指数位中
e的取值范围是 e∈[0, 255]，又因为指数必须可以取到小于零的数，b = e - 127，所以指数的范围 b∈[-127, 128]
a总共23位，a是一个二进制整数，一个二进制非零整数的最高有效位一定是1，1不需要显式的表达，所以a有24位的精度

假设符号位为s，指数位为e，有效数字位为m，则一个浮点数可以表示为
s * 1.m * 2^(e-127)

例如 6.5 = 1.101(base2) * 2^2，内存分布为e = 129，m = 10100000000000000000000

如果仅仅只有以上规则，float无法表示0，因为有效位数的隐藏位默认为1，为了避免这种情况，做出了如下规则
	1.如果指数位全都为0，那么隐藏位被解释为0
	2.如果指数位不全为0，那么隐藏位被解释为1
	具体信息请查看 https://www.jianshu.com/p/43b1b09f27f4

众所周知，计算机储存的浮点数不是连续的，是离散的，计算机中可表示的浮点数之间是有间隔的，间隔是多少？

在[2^e, 2^(e+1)]范围内，间隔为数值的变化量除以该范围内浮点数的个数
spacing = (2^(e+1) - 2^e) / 2^23 = 2^(e-23)

假设a与b都是浮点数，浮点数相加就会导致误差，a+b的误差范围是多少？
我们用 ⊕ 表示计算机浮点加法，假设误差为ε
r = a ⊕ b = round(a + b) , r ∈ [(a+b)*(1-ε), (a+b)*(1+ε)]

假设 a+b = c * 2^e，有效数字位为c，c∈[1,2)

该数所在的实数范围应该是 range ∈ [2^e, 2^(e+1))
对于float类型的数据，有效位数为23位，则range区间被平均分为2^23个，每个子区间是长度为ulp
ulp = (2^(e+1) - 2^e) / 2^23 = 2^(e-23)，

            |<---ulp--->|<---ulp--->|
            |___________|___________|
                              p

如上所示，我们定义绝对误差为 ae, 当实数位于ulp的中点时，绝对误差最大ae = ulp / 2 = 2^-24

只要指数位不变，ulp不变，ae不变，我们定义相对误差为re，绝对误差比当前数值，re = ae/(a+b)

当c=1时，a+b取到最小值，相对误差re取到最大值ε, ε = 2^-24

由此可分析出绝对误差范围
a ⊕ b = round(a + b) ∈ [(a+b)*(1-ε), (a+b)*(1+ε)]
a ⊖ b = round(a - b) ∈ [(a-b)*(1-ε), (a-b)*(1+ε)]
a ⊗ b = round(a * b) ∈ [(a*b)*(1-ε), (a*b)*(1+ε)]
a ⊘ b = round(a / b) ∈ [(a/b)*(1-ε), (a/b)*(1+ε)]
sqrt(a) = round(√a) ∈ [(√a)*(1-ε), (√a)*(1+ε)]
其中 ε = 2^-24

 */

//inline bool quadratic(EFloat A, EFloat B, EFloat C, EFloat *t0, EFloat *t1);

class EFloat {
public:
    EFloat() {}
    EFloat(float _val, float err = 0.f) : _val(_val) {
        if (err == 0.)
            _low = _high = _val;
        else {
            _low = nextFloatDown(_val - err);
            _high = nextFloatUp(_val + err);
        }
       
#ifndef NDEBUG
        _vPrecise = _val;
        check();
#endif  // NDEBUG
    }

#ifndef NDEBUG
    EFloat(float _val, long double lD, float err) : EFloat(_val, err) {
        _vPrecise = lD;
        check();
    }
#endif  // DEBUG

    // 加法，上界相加，下界相加
    EFloat operator + (const EFloat &ef) const {
        EFloat r;
        r._val = _val + ef._val;
#ifndef NDEBUG
        r._vPrecise = _vPrecise + ef._vPrecise;
#endif  // DEBUG
        r._low = nextFloatDown(lowerBound() + ef.lowerBound());
        r._high = nextFloatUp(upperBound() + ef.upperBound());
        r.check();
        return r;
    }

    // 强制类型转换
    explicit operator float() const { 
        return _val; 
    }

    // 强制类型转换
    explicit operator double() const { 
        return _val; 
    }

    float getAbsoluteError() const { 
        return _high - _low; 
    }

    float upperBound() const { 
        return _high; 
    }

    float lowerBound() const { 
        return _low; 
    }

#ifndef NDEBUG
    float getRelativeError() const {
        return std::abs((_vPrecise - _val) / _vPrecise);
    }
    long double preciseValue() const { 
        return _vPrecise; 
    }
#endif

    // 减法，要注意的是误差要保守估计
    EFloat operator - (const EFloat &ef) const {
        EFloat r;
        r._val = _val - ef._val;
#ifndef NDEBUG
        r._vPrecise = _vPrecise - ef._vPrecise;
#endif
        // 新的下界尽可能低
        r._low = nextFloatDown(lowerBound() - ef.upperBound());
        // 新的上界尽可能高
        r._high = nextFloatUp(upperBound() - ef.lowerBound());
        r.check();
        return r;
    }

    EFloat operator * (const EFloat &ef) const {
        EFloat r;
        r._val = _val * ef._val;
#ifndef NDEBUG
        r._vPrecise = _vPrecise * ef._vPrecise;
#endif
        Float prod[4] = {
            lowerBound() * ef.lowerBound(),
            upperBound() * ef.lowerBound(),
            lowerBound() * ef.upperBound(),
            upperBound() * ef.upperBound()};

        // 新的下界尽可能低
        r._low = nextFloatDown(std::min(std::min(prod[0], prod[1]), std::min(prod[2], prod[3])));
        // 新的上界尽可能高
        r._high = nextFloatUp(std::max(std::max(prod[0], prod[1]), std::max(prod[2], prod[3])));
        r.check();
        return r;
    }

    EFloat operator / (const EFloat &ef) const {
        EFloat r;
        r._val = _val / ef._val;
#ifndef NDEBUG
        r._vPrecise = _vPrecise / ef._vPrecise;
#endif
        if (ef._low < 0 && ef._high > 0) {
            r._low = -Infinity;
            r._high = Infinity;
        } else {
            Float di_val[4] = {
                lowerBound() / ef.lowerBound(), 
                upperBound() / ef.lowerBound(),
                lowerBound() / ef.upperBound(),
                upperBound() / ef.upperBound()};
            // 新的下界尽可能低
            r._low = nextFloatDown(std::min(std::min(di_val[0], di_val[1]), std::min(di_val[2], di_val[3])));
            // 新的上界尽可能高
            r._high = nextFloatUp(std::max(std::max(di_val[0], di_val[1]), std::max(di_val[2], di_val[3])));
        }
        r.check();
        return r;
    }

    EFloat operator - () const {
        EFloat r;
        r._val = -_val;
#ifndef NDEBUG
        r._vPrecise = -_vPrecise;
#endif
        r._low = -_high;
        r._high = -_low;
        r.check();
        return r;
    }

    inline bool operator == (const EFloat &fe) const {
        return _val == fe._val; 
    }

    inline void check() const {
        if (!std::isinf(_low) && !std::isnan(_low) && !std::isinf(_high) &&
            !std::isnan(_high))
            CHECK_LE(_low, _high);
#ifndef NDEBUG
        if (!std::isinf(_val) && !std::isnan(_val)) {
            CHECK_LE(lowerBound(), _vPrecise);
            CHECK_LE(_vPrecise, upperBound());
        }
#endif
    }

    EFloat(const EFloat &ef) {
        ef.check();
        _val = ef._val;
        _low = ef._low;
        _high = ef._high;
#ifndef NDEBUG
        _vPrecise = ef._vPrecise;
#endif
    }

    EFloat &operator = (const EFloat &ef) {
        ef.check();
        if (&ef != this) {
            _val = ef._val;
            _low = ef._low;
            _high = ef._high;
#ifndef NDEBUG
            _vPrecise = ef._vPrecise;
#endif
        }
        return *this;
    }
    
    friend std::ostream &operator<<(std::ostream &os, const EFloat &ef) {
        os << StringPrintf("_val=%f (%a) - [%f, %f]",
                           ef._val, ef._val, ef._low, ef._high);
#ifndef NDEBUG
        os << StringPrintf(", precise=%.30Lf", ef._vPrecise);
#endif // !NDEBUG
        return os;
    }
    
private:
    // 浮点数round之后的值
    float _val;

    // round之后减去绝对误差
    float _low;

    // round之后加上绝对误差
    float _high;
#ifndef NDEBUG
    // debug模式下的高精度数据
    long double _vPrecise;
#endif  // NDEBUG
    friend inline EFloat sqrt(const EFloat &fe);
    friend inline EFloat abs(const EFloat &fe);
    friend inline bool quadratic(const EFloat &A, const EFloat &B, const EFloat &C, EFloat *t0, EFloat *t1);
};

inline EFloat operator*(float f, EFloat fe) {
    return EFloat(f) * fe; 
}

inline EFloat operator/(float f, EFloat fe) {
    return EFloat(f) / fe; 
}

inline EFloat operator+(float f, EFloat fe) {
    return EFloat(f) + fe; 
}

inline EFloat operator-(float f, EFloat fe) {
    return EFloat(f) - fe; 
}

inline EFloat sqrt(const EFloat &fe) {
    EFloat r;
    r._val = std::sqrt(fe._val);
#ifndef NDEBUG
    r._vPrecise = std::sqrt(fe._vPrecise);
#endif
    r._low = nextFloatDown(std::sqrt(fe._low));
    r._high = nextFloatUp(std::sqrt(fe._high));
    r.check();
    return r;
}

inline EFloat abs(const EFloat &fe) {
    if (fe._low >= 0)
        // 如果下界大于零，直接返回
        return fe;
    else if (fe._high <= 0) {
        // 如果上界小于零，则每个成员都乘以-1
        EFloat r;
        r._val = -fe._val;
#ifndef NDEBUG
        r._vPrecise = -fe._vPrecise;
#endif
        r._low = -fe._high;
        r._high = -fe._low;
        r.check();
        return r;
    } else {
        EFloat r;
        r._val = std::abs(fe._val);
#ifndef NDEBUG
        r._vPrecise = std::abs(fe._vPrecise);
#endif
        r._low = 0;
        r._high = std::max(-fe._low, fe._high);
        r.check();
        return r;
    }
}

inline bool quadratic(const EFloat &A, const EFloat &B, const EFloat &C, EFloat *t0, EFloat *t1) {
    // ax^2 + bx + c = 0
    // 先计算判别式
    double discrim = (double)B._val * (double)B._val - 4. * (double)A._val * (double)C._val;
    if (discrim < 0.) {
        return false;
    }
    double rootDiscrim = std::sqrt(discrim);
    
    EFloat floatRootDiscrim(rootDiscrim, MachineEpsilon * rootDiscrim);
    
    EFloat q;
    if ((float)B < 0) {
        q = -.5 * (B - floatRootDiscrim);
    } else {
        q = -.5 * (B + floatRootDiscrim);
    }
    *t0 = q / A;
    *t1 = C / q;
    if ((float)*t0 > (float)*t1) {
        std::swap(*t0, *t1);
    }
    return true;
}


PALADIN_END

#endif /* errfloat_h */
