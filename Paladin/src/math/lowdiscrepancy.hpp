//
//  lowdiscrepancy.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/7/25.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef lowdiscrepancy_hpp
#define lowdiscrepancy_hpp

#include "header.h"

#include "sampling.hpp"

PALADIN_BEGIN

// 质数表大小
static CONSTEXPR int PrimeTableSize = 1000;
// 质数表
extern const int Primes[PrimeTableSize];
// 质数和，i与v的关系为前i个质数相加，值为v
extern const int PrimeSums[PrimeTableSize];

// 逐位反转
Float RadicalInverse(int baseIndex, uint64_t a);

Float ScrambledRadicalInverse(int baseIndex, uint64_t a, const uint16_t *perm);


/**
 * 反转函数
 * base 为进制数
 * @param  inverse 需要反转的数字
 * @param  nDigits 需要反转的前nDigits位
 * @return         返回 base进制数 inverse 的前 nDigits 位反转之后的数字
 * 例如，InverseRadicalInverse<10>(1234567, 3)的返回值为765
 */
template <int base>
inline uint64_t InverseRadicalInverse(uint64_t inverse, int nDigits) {
    uint64_t index = 0;
    for (int i = 0; i < nDigits; ++i) {
        uint64_t digit = inverse % base;
        inverse /= base;
        index = index * base + digit;
    }
    return index;
}

/**
 * 计算随机排列表
 * 为排列表初始化一个连续的数组
 */
std::vector<uint16_t> ComputeRadicalInversePermutations(RNG &rng);

inline uint32_t ReverseBits32(uint32_t n) {
    n = (n << 16) | (n >> 16);
    n = ((n & 0x00ff00ff) << 8) | ((n & 0xff00ff00) >> 8);
    n = ((n & 0x0f0f0f0f) << 4) | ((n & 0xf0f0f0f0) >> 4);
    n = ((n & 0x33333333) << 2) | ((n & 0xcccccccc) >> 2);
    n = ((n & 0x55555555) << 1) | ((n & 0xaaaaaaaa) >> 1);
    return n;
}

inline uint64_t ReverseBits64(uint64_t n) {
    uint64_t n0 = ReverseBits32((uint32_t)n);
    uint64_t n1 = ReverseBits32((uint32_t)(n >> 32));
    return (n0 << 32) | n1;
}

PALADIN_END

#endif /* lowdiscrepancy_hpp */
