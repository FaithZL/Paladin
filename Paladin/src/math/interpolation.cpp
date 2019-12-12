//
//  interpolation.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/12/6.
//

#include "interpolation.hpp"

PALADIN_BEGIN

/**
 * [CatmullRom 样条插值]
 * @param  size   数组大小
 * @param  nodes  离散横坐标列表
 * @param  values 纵坐标列表
 * @param  x      需要插值的x值
 * @return        
 */
Float CatmullRom(int size, const Float *nodes, const Float *values, Float x) {
    if (x < nodes[0] || x > nodes[size - 1]) {
    	return 0;
    }
    int idx = findInterval(size, [&](int i) { return nodes[i] <= x; });
    Float x0 = nodes[idx], x1 = nodes[idx + 1];
    Float f0 = values[idx], f1 = values[idx + 1];
    Float width = x1 - x0;
    Float d0, d1;
    // 处理左边界情况
    if (idx > 0) {
        d0 = width * (f1 - values[idx - 1]) / (x1 - nodes[idx - 1]);
    } else {
    	// 如果是左边界情况，导函数直接用f(1)-f(0)代替
    	// todo这里为何不乘以width
        d0 = f1 - f0; 
    }

    // 处理右边界情况
    if (idx + 2 < size) {
        d1 = width * (values[idx + 2] - f0) / (nodes[idx + 2] - x0);
    } else {
    	// todo这里为何不乘以width
        d1 = f1 - f0;
    }

    // 线性插值算出t，t∈[0,1]
    Float t = (x - x0) / (x1 - x0), t2 = t * t, t3 = t2 * t;
    return (2 * t3 - 3 * t2 + 1) * f0 
    		+ (-2 * t3 + 3 * t2) * f1
    		+ (t3 - 2 * t2 + t) * d0 
    		+ (t3 - t2) * d1;
}

bool CatmullRomWeights(int size, const Float *nodes, Float x, int *offset,
                       Float *weights) {
	if (x < nodes[0] || x > nodes[size - 1]) {
    	return false;
    }

	int idx = findInterval(size, [&](int i) { return nodes[i] <= x; });
	*offset = idx - 1;
	Float x0 = nodes[idx];
	Float x1 = nodes[idx + 1];
	// 线性插值算出t，t∈[0,1]
	Float t = (x - x0) / (x1 - x0), t2 = t * t, t3 = t2 * t;
	weights[1] = 2 * t3 - 3 * t2 + 1;
	weights[2] = -2 * t3 + 3 * t2;
	Float width = x1 - x0;

	// 处理左边界情况
	if (idx > 0) {
		Float w0 = (t3 - 2 * t2 + t) * width / (x1 - nodes[idx - 1]);
		weights[0] = -w0;
        weights[2] += w0;
	} else {
		// 如果是左边界情况，导函数直接用f(1)-f(0)代替
        Float w0 = t3 - 2 * t2 + t;
        weights[0] = 0;
        weights[1] -= w0;
        weights[2] += w0;
	}

	// 处理右边界情况
	if (idx + 2 < size) {
        Float w3 = (t3 - t2) * width / (nodes[idx + 2] - x0);
        weights[1] -= w3;
        weights[3] = w3;
    } else {
        Float w3 = t3 - t2;
        weights[1] -= w3;
        weights[2] += w3;
        weights[3] = 0;
    }
    return true;
}

/**
 * 采样CatmullRom曲线
 * @param  n    样本数量
 * @param  x    横坐标列表
 * @param  f    纵坐标列表
 * @param  F    积分上限函数
 * @param  u    均匀变量
 * @param  fval 导函数值
 * @param  pdf  概率密度函数值
 * @return      [description]
 */
Float SampleCatmullRom(int n, const Float *x, const Float *f, const Float *F,
                       Float u, Float *fval, Float *pdf) {
	// 由于积分上限函数F没有归一化，所以...
	// Fi ≤ ξ1 * 1Fn−1 ≤ Fi+1,
	u *= F[n - 1];
	int i = findInterval(n, [&](int i) { return F[i] <= u; });

	Float x0 = x[i];
	Float x1 = x[i + 1];
	Float f0 = f[i];
	Float f1 = f[i + 1];

	Float width = x1 - x0;

	Float d0;
	Float d1;

	if (i > 0) {
		d0 = width * (f1 - f[i - 1]) / (x1 - x[i - 1]);
	} else {
		d0 = f1 - f0;
	}

	if (i + 2 < n) {
		d1 = width * (f[i + 2] - f0) / (x[i + 2] - x0);
	} else {
		d1 = f1 - f0;
	}
	// 找到u所在的子区间之后重新映射
	// x = F^-1(ξ1 * Fn−1 − Fi)
	u = (u - F[i]) / width;

	// 开始使用牛顿-二分法来求定义域值
    // 首先先要选择一个合适的初始值,这里假设样条线段是线性的 f(t) = (1-t) f(0) + tf(1)
    // 然后求积分 F(t) = tf(0) - t^2 * f(0)+t^2 * f(1)
    // 然后求逆函数 f(0)-srqt(f(0)^2 + 2(f(1)-f(0))*x) / (f(0)-f(1))
	Float t;
	if (f0 != f1) {
		t = (f0 - std::sqrt(std::max((Float)0, f0 * f0 + 2 * u * (f1 - f0)))) 
		    / (f0 - f1);
	} else {
		t = u / f0;
	}

	Float a = 0, b = 1, Fhat, fhat;
	while (true) {
		if (t <= a || t >= b) {
			t = 0.5f * (a + b);
		}

		// 计算相应的t的函数值
        // 这里是相应的CatmullRom样条的Horner形式
        // horner形式的好处是效率高
        // 简单来说就是
        // x^5 + x^4 + x^3 + x^2 + x
        // 可以转换为
        // x*(x*(x*(x*(x + 1) + 1) + 1) + 1)形式
		Fhat = t * (f0 +
                    t * (.5f * d0 +
                         t * ((1.f / 3.f) * (-2 * d0 - d1) + f1 - f0 +
                              t * (.25f * (d0 + d1) + .5f * (f0 - f1)))));
        Fhat = f0 +
               t * (d0 +
                    t * (-2 * d0 - d1 + 3 * (f1 - f0) +
                         t * (d0 + d1 + 2 * (f0 - f1))));

        if (std::abs(Fhat - u) < 1e-6f || b - a < 1e-6f) {
        	break;
        }

		if (Fhat < u) {
            a = t;
		} else {
            b = t;
		}

        t -= (Fhat - u) / fhat;
	}

	if (fval) {
		*fval = fhat;
	}
    if (pdf) {
    	*pdf = fhat / F[n - 1];
    }
    return x0 + width * t;
}

/**
 * 随机采样f(α, x),
 * @param  size1  第一维度的样本数量
 * @param  size2  第二维度的样本数量
 * @param  nodes1 第一维度的样本列表
 * @param  nodes2 第二维度的样本列表
 * @param  values 函数值列表，二维数组，row major
 * @param  cdf    一个二维离散CDF矩阵，每一行通过IntegrateCatmullRom函数计算而来
 * @param  alpha  固定参数，可以是bssrdf反射率，也可以是fourierBSDF的cosθ
 * @param  u	  [description]
 * @param  fval   [description]
 * @param  pdf    [description]
 * @return        [description]
 */
Float SampleCatmullRom2D(int size1, int size2, const Float *nodes1,
                         const Float *nodes2, const Float *values,
                         const Float *cdf, Float alpha, Float u, Float *fval,
                         Float *pdf) {
	int offset;
    Float weights[4];
    // 计算出alpha对应nodes1列表中的偏移，与权重
    // offset表示第offset行
    if (!CatmullRomWeights(size1, nodes1, alpha, &offset, weights)) {
    	return 0;
    }

	auto interpolate = [&](const Float *array, int idx) {
        Float value = 0;
        for (int i = 0; i < 4; ++i) {
            if (weights[i] != 0) {
                value += array[(offset + i) * size2 + idx] * weights[i];
            }
        }
        return value;
    };
    // 找到当前alpha所在的行的最大值
	Float maximum = interpolate(cdf, size2 - 1);
	// 不解释，你懂的
    u *= maximum;
	int idx = findInterval(size2, [&](int i) {return interpolate(cdf, i) <= u;});

	Float f0 = interpolate(values, idx);
	Float f1 = interpolate(values, idx + 1);
    Float x0 = nodes2[idx], x1 = nodes2[idx + 1];
    Float width = x1 - x0;
    Float d0, d1;

    // 找到u所在的子区间之后重新映射
	// x = F^-1(ξ1 * Fn−1 − Fi)
    u = (u - interpolate(cdf, idx)) / width;

    if (idx > 0) {
    	// 中心差分
		d0 = width * (f1 - interpolate(values, idx - 1)) /
             (x1 - nodes2[idx - 1]);
    } else {
    	d0 = f1 - f0;
    }

    if (idx + 2 < size2) {
    	// 中心差分
		d1 = width * (interpolate(values, idx + 2) - f0) /
             (nodes2[idx + 2] - x0);
    } else {
    	d1 = f1 - f0;
    }

	// 开始使用牛顿-二分法来求定义域值
    // 首先先要选择一个合适的初始值,这里假设样条线段是线性的 f(t) = (1-t) f(0) + tf(1)
    // 然后求积分 F(t) = tf(0) - t^2 * f(0)+t^2 * f(1)
    // 然后求逆函数 f(0)-srqt(f(0)^2 + 2(f(1)-f(0))*x) / (f(0)-f(1))
	Float t;
    if (f0 != f1) {
        t = (f0 - std::sqrt(std::max((Float)0, f0 * f0 + 2 * u * (f1 - f0)))) /
            (f0 - f1);
    } else {
        t = u / f0;
    }
    Float a = 0, b = 1, Fhat, fhat;

    // 牛顿二分迭代法
	while (true) {
        if (t < a || t > b) {
        	t = 0.5f * (a + b);
        }

        // 原函数的horner形式
        Fhat = t * (f0 +
                    t * (.5f * d0 +
                         t * ((1.f / 3.f) * (-2 * d0 - d1) + f1 - f0 +
                              t * (.25f * (d0 + d1) + .5f * (f0 - f1)))));
        // 导函数的horner形式
        fhat = f0 +
               t * (d0 +
                    t * (-2 * d0 - d1 + 3 * (f1 - f0) +
                         t * (d0 + d1 + 2 * (f0 - f1))));

        // 收敛到一个较小的值
        if (std::abs(Fhat - u) < 1e-6f || b - a < 1e-6f) {
        	break;
        }

        if (Fhat < u) {
            a = t;
        } else {
            b = t;
        }

        // 执行牛顿迭代
        t -= (Fhat - u) / fhat;
    }

    if (fval) {
    	*fval = fhat;
    }
    if (pdf) {
    	*pdf = fhat / maximum;
    }
    return x0 + width * t;
}

/**
 * 对CatmullRom曲线求积分
 * 
 *                                      f(xi) + f(x_i+1)     f'(xi) - f'(x_i+1)
 * ∫[xi, x_i+1]pi(x)dx = (x_i+1 - xi) (------------------ + --------------------)
 *                                             2                     12
 *
 * 恕我直言，这个表达式我还真不知道是怎么推导出来的todo
 * 根据书上的描述
 * 似乎应该可以用interpolation.hpp中的3式推导出来，搞完主线之后回头试试看
 * 
 * @param  n      样本数量
 * @param  x      样本横坐标
 * @param  values 样本纵坐标
 * @param  cdf    返回值，累积分布函数(不保证归一化)
 * @return        返回积分值
 */
Float IntegrateCatmullRom(int n, const Float *x, const Float *values,
                          Float *cdf)  {
	Float sum = 0;
	cdf[0] = 0;
	for (int i = 0; i < n - 1; ++i) {
		Float x0 = x[i];
		Float x1 = x[i + 1];
		Float f0 = values[i];
		Float f1 = values[i + 1];
		Float width = x1 - x0;

		Float d0, d1;
		if (i > 0) {
			d0 = width * (f1 - values[i - 1]) / (x1 - x[i - 1]);
		} else {
			d0 = f1 - f0;
		}

		if (i + 2 < n) {
			d1 = width * (values[i + 2] - f0) / (x[i + 2] - x0);
		} else {
			d1 = f1 - f0;
		}

		sum += ((d0 - d1) * (1.f / 12.f) + (f0 + f1) * .5f) * width;
        cdf[i + 1] = sum;
	}
	return sum;
}

/**
 * catmull rom 反函数
 * @param  n      样本数量
 * @param  x      样本横坐标列表
 * @param  values 样本纵坐标列表
 * @param  u      
 * @return        
 */
Float InvertCatmullRom(int n, const Float *x, const Float *values, Float u)  {
	if (u <= values[0]) {
		return x[0];
	} else if (u >= values[n - 1]) {
		return x[n - 1];
	}

	// 找到对应u的索引
	int i = findInterval(n, [&](int i) { return values[i] <= u; });
	Float x0 = x[i];
	Float x1 = x[i + 1];
	Float f0 = values[i];
	Float f1 = values[i + 1];
	Float width = x1 - x0;

	Float d0, d1;
	if (i > 0) {
		d0 = width * (f1 - values[i - 1]) / (x1 - x[i - 1]);
	} else {
		d0 = f1 - f0;
	}

	if (i + 2 < n) {
		d1 = width * (values[i + 2] - f0) / (x[i + 2] - x0);
	} else {
		d1 = f1 - f0;
	}

	// 牛顿迭代法，详见animatedtransform.hpp
	// 在区间[0,1]内找到纵坐标u对应的横坐标
	Float a = 0, b = 1, t = .5f;
	// 函数值
    Float Fhat;
	// 导函数值
	Float fhat;
	while (true) {
		if (t <= a || t >= b) {
			t = 0.5f * (a + b);
		}

		Float t2 = t * t, t3 = t2 * t;

		Fhat = (2 * t3 - 3 * t2 + 1) * f0 
			+ (-2 * t3 + 3 * t2) * f1 
			+ (t3 - 2 * t2 + t) * d0 
			+ (t3 - t2) * d1;

		fhat = (6 * t2 - 6 * t) * f0 
			+ (-6 * t2 + 6 * t) * f1 
			+ (3 * t2 - 4 * t + 1) * d0 
			+ (3 * t2 - 2 * t) * d1;

		if (std::abs(Fhat - u) < 1e-6f || b - a < 1e-6f) {
			break;
		}

		if (Fhat - u < 0) {
            a = t;
		} else {
            b = t;
        }

        t -= (Fhat - u) / fhat;
	}

	return x0 + t * width;
}

PALADIN_END
