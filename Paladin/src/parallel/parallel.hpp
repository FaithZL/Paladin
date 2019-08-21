//
//  parallel.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/8/21.
//

#ifndef parallel_hpp
#define parallel_hpp

#include "header.h"

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
    void Add(Float v) {
#ifdef PALADIN_FLOAT_AS_DOUBLE
        uint64_t oldBits = bits, newBits;
#else
        uint32_t oldBits = _bits, newBits;
#endif
        do {
            newBits = floatToBits(bitsToFloat(oldBits) + v);
        } while (!_bits.compare_exchange_weak(oldBits, newBits));
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
