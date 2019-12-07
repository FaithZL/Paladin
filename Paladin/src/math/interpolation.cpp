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
        d0 = f1 - f0;
    }

    // 处理右边界情况
    if (idx + 2 < size) {
        d1 = width * (values[idx + 2] - f0) / (nodes[idx + 2] - x0);
    } else {
        d1 = f1 - f0;
    }

    // 线性插值算出t
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
	// 线性插值计算t
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

PALADIN_END
