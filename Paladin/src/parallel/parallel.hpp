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

//原子操作的Float类型
//c++不支持原子的Float操作
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
        // 如果oldBits与_bits不相等，则把oldBits改为_bits的值，返回false
        // 如果oldBits与_bits相等，则把_bit的值改为newBits，返回true
    }
    
private:
    // C++11给我们带来的Atomic一系列原子操作类，它们提供的方法能保证具有原子性。
    // 这些方法是不可再分的，获取这些变量的值时，永远获得修改前的值或修改后的值，不会获得修改过程中的中间数值
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

void parallelFor(std::function<void(int64_t)> func, int64_t count, int chunkSize = 1);

extern thread_local int ThreadIndex;

void parallelFor2D(std::function<void(Point2i)> func, const Point2i &count);

int maxThreadIndex();

int numSystemCores();

void parallelInit();

void parallelCleanup();

void mergeWorkerThreadStats();

PALADIN_END

#endif /* parallel_hpp */
