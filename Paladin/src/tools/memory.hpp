//
//  memory.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/7/17.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef memory_hpp
#define memory_hpp

#include "header.h"
#include <list>
#include <cstddef>

PALADIN_BEGIN

// Memory Declarations
#define ARENA_ALLOC(arena, Type) new ((arena).alloc(sizeof(Type))) Type
void *allocAligned(size_t size);
template <typename T>
T *allocAligned(size_t count) {
    return (T *)allocAligned(count * sizeof(T));
}

void freeAligned(void *);

/*
 * 内存管理是一个很复杂的问题，但在离线渲染器中，内存管理的情况相对简单，大部分的内存申请
 * 主要集中在解析场景的阶段，这些内存在渲染结束之前一直被使用
 * 为何要使用内存池？
 * 
 * 1.频繁的new跟delete性能消耗很高，new运算符执行的时候相当于会使当前线程block，
 * 直到操作系统返回可用内存时，线程才继续执行，如果使用了内存池，预先申请一大块连续内存
 * 之后每次申请内存时不是向操作系统申请，而是直接将指向当前地址的指针自增就可以了，分配效率高
 * 
 * 2.用内存池可以自定义内存对齐的方式，从而写出对缓存友好的程序
 *     好的对齐方式可以提高缓存命中率，比如CPU从内存中将数据加载到缓存中时
 *     会从特定的地址(必须是cache line长度的整数倍)中加载特定的长度(必须是cache line的长度)
 *     通常cache line的长度为64字节，如果一个int所占的位置横跨了两个cache line，cache miss最多为两次
 *     如果该数据的完全在一个cache line以内，那么cache miss的次数最多为一次
 *
 */
class alignas(PALADIN_L1_CACHE_LINE_SIZE) MemoryArena {

public:

    /**
     * MemoryArena是内存池的一种，基于arena方式分配内存
     * 整个对象大体分为三个部分
     *   1.可用列表，用于储存可分配的内存块
     *   2.已用列表，储存已经使用的内存块
     *   3.当前内存块
     */
    MemoryArena(size_t blockSize = 262144) : _blockSize(blockSize) {

    }

    ~MemoryArena() {
        freeAligned(_currentBlock);

        for (auto &block : _usedBlocks) {
            freeAligned(block.second);
        }

        for (auto &block : _availableBlocks) {
            freeAligned(block.second);
        }
    }

    /**
     * 1.对齐内存块
     * 2.
     */
    void * alloc(size_t nBytes) {

        // 16位对齐，对齐之后nBytes为16的整数倍
        nBytes = (nBytes + 15) & ~(15);
        if (_currentBlockPos + nBytes > _currentAllocSize) {
            // 如果已经分配的内存加上请求内存大于当前内存块大小
            
            if (_currentBlock) {
                // 如果当前块不为空，则把当前块放入已用列表中
                _usedBlocks.push_back(std::make_pair(_currentAllocSize, _currentBlock));
                _currentBlock = nullptr;
                _currentAllocSize = 0;
            }
            
            // 在可用列表中查找是否有尺寸大于请求内存的块
            for (auto iter = _availableBlocks.begin(); iter != _availableBlocks.end(); ++iter) {
                if (iter->first >= nBytes) {
                    // 如果找到将当前块指针指向该块，并将该块从可用列表中移除
                    _currentAllocSize = iter->first;
                    _currentBlock = iter->second;
                    _availableBlocks.erase(iter);
                    break;
                }
            }

            if (!_currentBlock) {
                // 如果没有找到符合标准的内存块，则申请一块内存
                _currentAllocSize = std::max(nBytes, _blockSize);
                _currentBlock = allocAligned<uint8_t>(_currentAllocSize);
            }
            _currentBlockPos = 0;
        }
        void * ret = _currentBlock + _currentBlockPos;
        _currentBlockPos += nBytes;
        return ret;
    }

    /**
     * 外部向内存池申请一块连续内存
     */
    template <typename T>
    T * alloc(size_t n = 1, bool runConstructor = true) {
        T *ret = (T *)alloc(n * sizeof(T));
        if (runConstructor) {
            for (size_t i = 0; i < n; ++i) {
                new (&ret[i]) T();
            }
        }
        return ret;
    }

    /**
     * 重置当前内存池，将可用列表与已用列表合并，已用列表在可用列表之前
     */
    void reset() {
        _currentBlockPos = 0;
        _availableBlocks.splice(_availableBlocks.begin(), _usedBlocks);
    }

    //获取已经分配的内存大小
    size_t totalAllocated() const {
        size_t total = _currentAllocSize;
        for (const auto &alloc : _usedBlocks) {
            total += alloc.first;
        }
        for (const auto &alloc : _availableBlocks) {
            total += alloc.first;
        }
        return total;
    }
    
private:
    MemoryArena(const MemoryArena &) = delete;
    MemoryArena &operator=(const MemoryArena &) = delete;

    // 默认内存块大小
    const size_t _blockSize;

    // 当前块已经分配的位置
    size_t _currentBlockPos = 0;

    // 当前块的尺寸
    size_t _currentAllocSize = 0;

    // 当前块指针
    uint8_t *_currentBlock = nullptr;

    // 已经使用的内存块列表
    std::list<std::pair<size_t, uint8_t *>> _usedBlocks;

    // 可使用的内存块列表
    std::list<std::pair<size_t, uint8_t *>> _availableBlocks;
};


/**
 * 2D数组优化，重新排布内存结构
 * 二维数组在内存中的排布实际上是1维连续的
 * 假设二维数组两个维度分别为uv，当索引a[u][v]时，编译器实际上会转成一维数组的索引方式
 * idx = u * width + v
 * 
 * BlockedArray有何好处？
 * 数组的默认内存结构，a[u][v]被访问之后，通常周围的内存块容易被访问到
 * 比如说a[u + 1][v]，a[u - 1][v]但这这两个元素通常跟a[u][v]不在一个cache line上，导致cache miss
 * 
 * 为了优化目前这个缺点，基本思路，将一连串内存地址分隔为一个个块，块的尺寸为2的整数次幂，每个块逐行排布
 * 
 * 假设
 * logBlockSize = 2 ，blockSize = 4 每个块的长宽均为4
 * _uRes = 8
 * _vRes = 8
 * _uBlocks = roundUp(_uRes) >> logBlockSize = 2
 * nAlloc = roundUp(_uRes) * roundUp(_vRes) = 64 需要申请64个单位的内存空间
 * 
 * 如上参数，内存排布如下，上部分是实际内存中连续地址0-63，下部分是经过下标(u,v)重新映射之后索引的地址，
 * 可以看到，经过重新映射之后,(0,0)与(1,0)是相邻的
 * 
 * 整个内存片段分为2x2个block，每两个块一行
 * 
 * 基本思路是先找到(u,v)所在的块的索引的offset，然后找到块内的索引的offset
 *
 * 
 * |----------0 block----------| |----------1 block----------|
 * 
 * |__0___|__1___|__2___|__3___| |__16__|__17__|__18__|__19__|
 * |_0,0__|_1,0__|_2,0__|_3,0__| |_4,0__|_5,0__|_6,0__|_7,0__|
 * 
 * |__4___|__5___|__6___|__7___| |__20__|__21__|__22__|__23__|
 * |__0,1_|_1,1__|_2,1__|_3,1__| |_4,1__|_5,1__|_6,1__|_7,1__|
 * 
 * |___8__|___9__|__10__|__11__| |__24__|__25__|__26__|__27__|
 * |__0,2_|__1,2_|_2,2__|_3,2__| |_4,2__|_5,2__|_6,2__|_7,2__|
 * 
 * |__12__|__13__|__14__|__15__| |__28__|__29__|__30__|__31__|
 * |_0,3__|__1,3_|_2,3__|_3,3__| |_4,3__|_5,3__|_6,3__|_7,3__|
 * 
 * |----------2 block----------| |----------3 block----------|
 * 
 * |__32__|__33__|__34__|__35__| |__48__|__49__|__50__|__51__|
 * |__0,4_|_1,4__|_2,4__|_3,4__| |__4,4_|_5,4__|_6,4__|_7,4__|
 * 
 * |__36__|__37__|__38__|__39__| |__52__|__53__|__54__|__55__|
 * |_0,5__|_1,5__|_2,5__|_3,5__| |_4,5__|_5,5__|_6,5__|_7,5__|
 * 
 * |__40__|__41__|__42__|__43__| |__56__|__57__|__58__|__59__|
 * |_0,6__|_1,6__|_2,6__|_3,6__| |_4,6__|_5,6__|_6,6__|_7,6__|
 * 
 * |__44__|__45__|__46__|__47__| |__60__|__61__|__62__|__63__| 
 * |__0,7_|_1,7__|_2,7__|_3,7__| |_4,7__|_5,7__|_6,7__|_7,7__| 
 * 
 */
template <typename T, int logBlockSize>
class BlockedArray {
public:
    /*
    分配一段连续的内存块，用uv参数重排二维数组的索引
     */
    BlockedArray(int uRes, int vRes, const T *d = nullptr)
    : _uRes(uRes), 
    _vRes(vRes), 
    _uBlocks(roundUp(uRes) >> logBlockSize) {
        // 先向上取到2^logBlockSize
        int nAlloc = roundUp(_uRes) * roundUp(_vRes);
        _data = allocAligned<T>(nAlloc);
        for (int i = 0; i < nAlloc; ++i) {
 * new (&_data[i]) T();
        }
        if (d) {
            for (int v = 0; v < _vRes; ++v){
                for (int u = 0; u < _uRes; ++u) {
                    (*this)(u, v) = d[v * _uRes + u];
                }
            }
        }
    }

    /**
     * 2^logBlockSize
     */
    CONSTEXPR int blockSize() const { 
        return 1 << logBlockSize; 
    }

    /**
     * 向上取到最近的2的logBlockSize次幂
     */
    int roundUp(int x) const {
        return (x + blockSize() - 1) & ~(blockSize() - 1);
    }

    int uSize() const { 
        return _uRes; 
    }

    int vSize() const { 
        return _vRes; 
    }

    ~BlockedArray() {
        for (int i = 0; i < _uRes * _vRes; ++i) {
            _data[i].~T();
        }
        freeAligned(_data);
    }

    /**
     * 返回a * 2^logBlockSize
     */
    int block(int a) const { 
        return a >> logBlockSize; 
    }

    int offset(int a) const { 
        return (a & (blockSize() - 1)); 
    }

    inline int getTotalOffset(int u, int v) {
        int bu = block(u); 
        int bv = block(v);
        int ou = offset(u);
        int ov = offset(v);
        // 小block的偏移 
        int offset = blockSize() * blockSize() * (_uBlocks * bv + bu);
        // 小block内的偏移
        offset += blockSize() * ov + ou;
        return offset;
    }

    /**
     * 通过uv参数找到指定内存的思路
     * 1.先找到指定内存在哪个块中(bu,bv)
     * 2.然后找到块中的偏移量 (ou, ov)
     */
    T &operator()(int u, int v) {
        int offset = getTotalOffset(u, v);
        return _data[offset];
    }

    const T &operator()(int u, int v) const {
        int offset = getTotalOffset(u, v);
        return _data[offset];
    }

    void getLinearArray(T *a) const {
        for (int v = 0; v < _vRes; ++v) {
            for (int u = 0; u < _uRes; ++u) {
                *a++ = (*this)(u, v);
            }
        }
    }
    
private:
    T *_data;
    const int _uRes, _vRes, _uBlocks;
};

PALADIN_END

#endif /* memory_hpp */

/**
 * getTotalOffset(0,0) = 0
 * getTotalOffset(0,1) = 4
 * getTotalOffset(0,2) = 8
 * getTotalOffset(0,3) = 12
 * getTotalOffset(0,4) = 32
 * getTotalOffset(0,5) = 36
 * getTotalOffset(0,6) = 40
 * getTotalOffset(0,7) = 44
 * getTotalOffset(1,0) = 1
 * getTotalOffset(1,1) = 5
 * getTotalOffset(1,2) = 9
 * getTotalOffset(1,3) = 13
 * getTotalOffset(1,4) = 33
 * getTotalOffset(1,5) = 37
 * getTotalOffset(1,6) = 41
 * getTotalOffset(1,7) = 45
 * getTotalOffset(2,0) = 2
 * getTotalOffset(2,1) = 6
 * getTotalOffset(2,2) = 10
 * getTotalOffset(2,3) = 14
 * getTotalOffset(2,4) = 34
 * getTotalOffset(2,5) = 38
 * getTotalOffset(2,6) = 42
 * getTotalOffset(2,7) = 46
 * getTotalOffset(3,0) = 3
 * getTotalOffset(3,1) = 7
 * getTotalOffset(3,2) = 11
 * getTotalOffset(3,3) = 15
 * getTotalOffset(3,4) = 35
 * getTotalOffset(3,5) = 39
 * getTotalOffset(3,6) = 43
 * getTotalOffset(3,7) = 47
 * getTotalOffset(4,0) = 16
 * getTotalOffset(4,1) = 20
 * getTotalOffset(4,2) = 24
 * getTotalOffset(4,3) = 28
 * getTotalOffset(4,4) = 48
 * getTotalOffset(4,5) = 52
 * getTotalOffset(4,6) = 56
 * getTotalOffset(4,7) = 60
 * getTotalOffset(5,0) = 17
 * getTotalOffset(5,1) = 21
 * getTotalOffset(5,2) = 25
 * getTotalOffset(5,3) = 29
 * getTotalOffset(5,4) = 49
 * getTotalOffset(5,5) = 53
 * getTotalOffset(5,6) = 57
 * getTotalOffset(5,7) = 61
 * getTotalOffset(6,0) = 18
 * getTotalOffset(6,1) = 22
 * getTotalOffset(6,2) = 26
 * getTotalOffset(6,3) = 30
 * getTotalOffset(6,4) = 50
 * getTotalOffset(6,5) = 54
 * getTotalOffset(6,6) = 58
 * getTotalOffset(6,7) = 62
 * getTotalOffset(7,0) = 19
 * getTotalOffset(7,1) = 23
 * getTotalOffset(7,2) = 27
 * getTotalOffset(7,3) = 31
 * getTotalOffset(7,4) = 51
 * getTotalOffset(7,5) = 55
 * getTotalOffset(7,6) = 59
 * getTotalOffset(7,7) = 63
 */