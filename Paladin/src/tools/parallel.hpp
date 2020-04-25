//
//  parallel.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/8/21.
//

#ifndef parallel_hpp
#define parallel_hpp

/**
 * 由于之前没接触过并发编程，所以整理一下关于并发编程的一些理解，加深理解程度
 *
 * 先想想并行程序运行时可能会遇到什么问题
 *     1.数据同步问题，如果两个线程同时修改一个变量，则可能导致数据不同步
 *     2.若干个线程并行执行一段逻辑，如果其中一个线程执行的逻辑执行完毕，
 *         那就需要等待其他线程执行完毕之后再执行之后的逻辑。
 *
 * 带着以上两个要解决的问题解说一下
 *     std::atomic模板与mutex大类是用于解决问题1
 *     condition_variable是用于解决问题2
 * 
 * 参考资料
 * https://www.cnblogs.com/huty/p/8516997.html
 * 并行机制简介
 * C++11给我们带来的Atomic一系列原子操作类，它们提供的方法能保证具有原子性。
 * 这些方法是不可再分的，获取这些变量的值时，永远获得修改前的值或修改后的值，不会获得修改过程中的中间数值
 * 使用方法为std::atomic<T>模板类
 * 
 * std::mutex，最基本的Mutex类。互斥锁
 *     lock()，调用线程将锁住该互斥量。线程调用该函数会发生下面 3 种情况：
 *         (1). 如果该互斥量当前没有被锁住，则调用线程将该互斥量锁住，直到调用 unlock之前，该线程一直拥有该锁。
 *        (2). 如果当前互斥量被其他线程锁住，则当前的调用线程被阻塞住。
 *        (3). 如果当前互斥量被当前调用线程锁住，则会产生死锁(deadlock)。
 *    unlock()， 解锁，释放对互斥量的所有权。
 *    try_lock()，尝试锁住互斥量，如果互斥量被其他线程占有，
 *        则当前线程也不会被阻塞。线程调用该函数也会出现下面 3 种情况，
 *        (1). 如果当前互斥量没有被其他线程占有，则该线程锁住互斥量，直到该线程调用 unlock 释放互斥量。
 *        (2). 如果当前互斥量被其他线程锁住，则当前调用线程返回 false，而并不会被阻塞掉。
 *        (3). 如果当前互斥量被当前调用线程锁住，则会产生死锁(deadlock)。
 *
 * std::recursive_mutex 介绍
 *   std::recursive_mutex 与 std::mutex 一样，也是一种可以被上锁的对象，
 *   但是和 std::mutex 不同的是，std::recursive_mutex 允许同一个线程对互斥量多次上锁（即递归上锁），
 *   来获得对互斥量对象的多层所有权，
 *   std::recursive_mutex 释放互斥量时需要调用与该锁层次深度相同次数的 unlock()，
 *   可理解为 lock() 次数和 unlock() 次数相同，除此之外，
 *   std::recursive_mutex 的特性和 std::mutex 大致相同。
 *
 * std::unique_lock 介绍
 *   与 Mutex RAII 相关，方便线程对互斥量上锁，但提供了更好的上锁和解锁控制。
 *
 * std::condition_variable 介绍
 *    std::condition_variable 提供了两种 wait() 函数。
 *    1.void wait (unique_lock<mutex>& lck);
 *    当前线程调用 wait() 后将被阻塞(此时当前线程应该获得了锁（mutex），不妨设获得锁 lck)，
 *    直到另外某个线程调用 notify_* 唤醒了当前线程。
 *    
 *    当 std::condition_variable 对象的某个 wait 函数被调用的时候，
 *    它使用 std::unique_lock(通过 std::mutex) 来锁住当前线程。当前线程会一直被阻塞，
 *    直到另外一个线程在相同的 std::condition_variable 对象上调用了 notification 函数来唤醒当前线程。
 *
 *    2.template <class Predicate>
 *      void wait (unique_lock<mutex>& lck, Predicate pred);
 *    只有当 pred 条件为 false 时调用 wait() 才会阻塞当前线程，
 *    并且在收到其他线程的通知后只有当 pred 为 true 时才会被解除阻塞。因此第二种情况类似以下代码：
 *    while (!pred()) wait(lck);
 */

#include "core/header.h"
#include <thread>

PALADIN_BEGIN

// C++11给我们带来的Atomic一系列原子操作类，它们提供的方法能保证具有原子性。
// 这些方法是不可再分的，获取这些变量的值时，永远获得修改前的值或修改后的值，不会获得修改过程中的中间数值
// 但c++不支持原子的Float操作
// 所以就封装了原子操作的Float类型
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

#ifdef PALADIN_FLOAT_AS_DOUBLE
    std::atomic<uint64_t> _bits;
#else
    std::atomic<uint32_t> _bits;
#endif
};

/**
 * 可以理解为栅栏
 * 保证一切准备就绪之后，所有子线程尽可能的同时开始工作
 */
class Barrier {
public:
    Barrier(int count) 
    : _count(count) { 
        CHECK_GT(_count, 0);
    }

    ~Barrier() { 
        CHECK_EQ(_count, 0); 
    }

    void wait() {
        std::unique_lock<std::mutex> lock(_mutex);
        CHECK_GT(_count, 0);
        if (--_count == 0) {
            _cv.notify_all();
        } else {
            _cv.wait(lock, [this] { return _count == 0; });
        }
    }
    
private:
    // 互斥锁
    std::mutex _mutex;
    // 当 std::condition_variable 对象的某个 wait 函数被调用的时候，
    // 它使用 std::unique_lock(通过 std::mutex) 来锁住当前线程。当前线程会一直被阻塞，
    // 直到另外一个线程在相同的 std::condition_variable 对象上调用了 notification 函数来唤醒当前线程。
    std::condition_variable _cv;
    int _count;
};

struct ParallelForLoop {
    
public:
    ParallelForLoop(std::function<void(int64_t)> func1D, int64_t maxIndex,
                    int chunkSize, uint64_t profilerState)
    : func1D(std::move(func1D)),
    maxIndex(maxIndex),
    chunkSize(chunkSize),
    profilerState(profilerState) {
        
    }
    
    ParallelForLoop(const std::function<void(Point2i, int)> &f, const Point2i &count,
                    uint64_t profilerState)
    : func2D(f),
    maxIndex(count.x * count.y),
    chunkSize(1),
    profilerState(profilerState) {
        numX = count.x;
    }
    
public:
    // 一维变量函数
    std::function<void(int64_t)> func1D;
    // 二维变量函数
    std::function<void(Point2i, int)> func2D;
    // 最大迭代次数
    const int64_t maxIndex;

    // 每次迭代的循环次数
    // 把n个任务分成若干块，每块的大小为chunkSize
    // 把n个任务分配给子线程也是按块分配
    const int chunkSize;

    uint64_t profilerState;
    // 下个迭代索引
    int64_t nextIndex = 0;
    // 当前激活worker线程
    int activeWorkers = 0;

    ParallelForLoop *next = nullptr;
    // 二维函数需要用到的属性
    int numX = -1;
    
    bool finished() const {
        return nextIndex >= maxIndex && activeWorkers == 0;
    }
};

extern thread_local int ThreadIndex;

void parallelFor(std::function<void(int64_t)> func, int64_t count, int chunkSize = 1);


void parallelFor2D(std::function<void(Point2i, int)> func, const Point2i &count);

inline int numSystemCores() {
    return std::max(1u, std::thread::hardware_concurrency());
}

int maxThreadIndex();

int getCurThreadIndex();

void setThreadNum(int num);

int getThreadNum();

void parallelInit(int num = 0);

void parallelCleanup();

void mergeWorkerThreadStats();

PALADIN_END

#endif /* parallel_hpp */
