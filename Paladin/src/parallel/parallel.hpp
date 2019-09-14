//
//  parallel.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/8/21.
//

#ifndef parallel_hpp
#define parallel_hpp

#include "core/header.h"

PALADIN_BEGIN

class AtomicFloat {
public:
    
    explicit AtomicFloat(Float v = 0) {
        _bits = floatToBits(v);
    }
    
    operator Float() const {
        return bitsToFloat(_bits);
    }
    
    Float operator=(Float v) {
        _bits = floatToBits(v);
        return v;
    }
    void add(Float v) {
#ifdef PALADIN_FLOAT_AS_DOUBLE
        uint64_t oldBits = _bits, newBits;
#else
        uint32_t oldBits = _bits, newBits;
#endif
        do {
            newBits = floatToBits(bitsToFloat(oldBits) + v);
        } while (!_bits.compare_exchange_weak(oldBits, newBits));
        // 当前值与期望值相等时，修改当前值为设定值，返回true
        // 当前值与期望值不等时，将期望值修改为当前值，返回false
    }
    
private:
    // AtomicFloat Private Data
#ifdef PALADIN_FLOAT_AS_DOUBLE
    std::atomic<uint64_t> _bits;
#else
    std::atomic<uint32_t> _bits;
#endif
};

PALADIN_END

#endif /* parallel_hpp */
