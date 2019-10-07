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
#ifdef PALADIN_FLOAT_AS_DOUBLE
    std::atomic<uint64_t> _bits;
#else
    std::atomic<uint32_t> _bits;
#endif
};

class Barrier {
public:
    Barrier(int count) 
    : _count(count) { 
        CHECK_GT(_count, 0);
    }

    ~Barrier() { 
        CHECK_EQ(_count, 0); 
    }

    void wait();
    
private:
    // 互斥锁
    std::mutex _mutex;
    // 当 std::condition_variable 对象的某个 wait 函数被调用的时候，
    // 它使用 std::unique_lock(通过 std::mutex) 来锁住当前线程。当前线程会一直被阻塞，
    // 直到另外一个线程在相同的 std::condition_variable 对象上调用了 notification 函数来唤醒当前线程。
    std::condition_variable _cv;
    int _count;
};

PALADIN_END

#endif /* parallel_hpp */
