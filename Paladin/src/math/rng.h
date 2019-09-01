//
//  rng.h
//  Paladin
//
//  Created by SATAN_Z on 2019/7/25.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef rng_h
#define rng_h

#include "header.h"

PALADIN_BEGIN

static const double DoubleOneMinusEpsilon = 0.99999999999999989;
static const float FloatOneMinusEpsilon = 0.99999994;

#ifdef FLOAT_AS_DOUBLE
static const Float OneMinusEpsilon = DoubleOneMinusEpsilon;
#else
static const Float OneMinusEpsilon = FloatOneMinusEpsilon;
#endif

#define PCG32_DEFAULT_STATE 0x853c49e6748fea9bULL
#define PCG32_DEFAULT_STREAM 0xda3e39cb94b95bdbULL
#define PCG32_MULT 0x5851f42d4c957f2dULL

/*
 PCG pseudo-random number generator
 伪随机发生器

 计算机中的伪随机数是有随机种子根据一定的算法生成的，所以，随机种子一定
 生成的随机序列也是一定的

 如果用户没有自行设定种子，默认情况下，随机种子来自于系统时间

 todo
 随机数算法确实比较复杂，暂时没搞懂原理
 */
class RNG {
    
public:
    RNG()
    : _state(PCG32_DEFAULT_STATE),
    _inc(PCG32_DEFAULT_STREAM) {
        
    }
    
    RNG(uint64_t sequenceIndex) {
        setSequence(sequenceIndex);
    }
    
    /*
     设置随机种子
     initseq为随机种子
     */
    void setSequence(uint64_t initseq) {
        _state = 0u;
        _inc = (initseq << 1u) | 1u;
        uniformUInt32();
        _state += PCG32_DEFAULT_STATE;
        uniformUInt32();
    }
    
    /*
     在[0, 2^32 - 1)区间上生成一个32位整型的伪随机数
     */
    uint32_t uniformUInt32() {
        uint64_t oldstate = _state;
        _state = oldstate * PCG32_MULT + _inc;
        uint32_t xorshifted = (uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
        uint32_t rot = (uint32_t)(oldstate >> 59u);
        return (xorshifted >> rot) | (xorshifted << ((~rot + 1u) & 31));
    }
    
    /*
     在[0, b - 1)区间上生成一个32位整型的伪随机数
     */    
    uint32_t uniformUInt32(uint32_t b) {
        uint32_t threshold = (~b + 1u) % b;
        while (true) {
            uint32_t r = uniformUInt32();
            if (r >= threshold) {
                return r % b;
            }
        }
    }
    
    /*
     在[0, 1)区间上生成一个浮点型伪随机数
     */    
    Float uniformFloat() {
        return std::min(OneMinusEpsilon, Float(uniformUInt32() * 2.3283064365386963e-10f));
    }
    
    template <typename Iterator>
    void shuffle(Iterator begin, Iterator end) {
        for (Iterator it = end - 1; it > begin; --it) {
            std::iter_swap(it, begin + uniformUInt32((uint32_t)(it - begin + 1)));
        }
    }
    
    void advance(int64_t idelta) {
        uint64_t cur_mult = PCG32_MULT, cur_plus = _inc, acc_mult = 1u,
        acc_plus = 0u, delta = (uint64_t)idelta;
        while (delta > 0) {
            if (delta & 1) {
                acc_mult *= cur_mult;
                acc_plus = acc_plus * cur_mult + cur_plus;
            }
            cur_plus = (cur_mult + 1) * cur_plus;
            cur_mult *= cur_mult;
            delta /= 2;
        }
        _state = acc_mult * _state + acc_plus;
    }
    
    int64_t operator-(const RNG &other) const {
        CHECK_EQ(_inc, other._inc);
        uint64_t cur_mult = PCG32_MULT, cur_plus = _inc, cur_state = other._state,
        the_bit = 1u, distance = 0u;
        while (_state != cur_state) {
            if ((_state & the_bit) != (cur_state & the_bit)) {
                cur_state = cur_state * cur_mult + cur_plus;
                distance |= the_bit;
            }
            CHECK_EQ(_state & the_bit, cur_state & the_bit);
            the_bit <<= 1;
            cur_plus = (cur_mult + 1ULL) * cur_plus;
            cur_mult *= cur_mult;
        }
        return (int64_t)distance;
    }
    
private:

    uint64_t _state;
    uint64_t _inc;
};

PALADIN_END





#endif /* rng_h */
