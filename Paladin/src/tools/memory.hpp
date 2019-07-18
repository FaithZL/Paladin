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
#define ARENA_ALLOC(arena, Type) new ((arena).Alloc(sizeof(Type))) Type
void *AllocAligned(size_t size);
template <typename T>
T *AllocAligned(size_t count) {
    return (T *)AllocAligned(count * sizeof(T));
}

void FreeAligned(void *);

/*
内存管理是一个很复杂的问题，但在离线渲染器中，内存管理的情况相对简单，大部分的内存申请
主要集中在解析场景的阶段，这些内存在渲染结束之前一直被使用
为何要使用内存池？

1.频繁的new跟delete性能消耗很高，new运算符执行的时候相当于会使当前线程block，
直到操作系统返回可用内存时，线程才继续执行，如果使用了内存池，预先申请一大块连续内存
之后每次申请内存时不是向操作系统申请，而是直接将指向当前地址的指针自增就可以了，分配效率高

2.用内存池可以自定义内存对齐的方式，从而写出对缓存友好的程序
    好的对齐方式可以提高缓存命中率，比如CPU从内存中将数据加载到缓存中时
    会从特定的地址(必须是cache line长度的整数倍)中加载特定的长度(必须是cache line的长度)
    通常cache line的长度为64字节，如果一个int所占的位置横跨了两个cache line，cache miss最多为两次
    如果该数据的完全在一个cache line以内，那么cache miss的次数最多为一次

*/
class alignas(PALADIN_L1_CACHE_LINE_SIZE) MemoryArena {
public:
    // MemoryArena Public Methods
    MemoryArena(size_t blockSize = 262144) : blockSize(blockSize) {}
    ~MemoryArena() {
        FreeAligned(currentBlock);
        for (auto &block : usedBlocks) FreeAligned(block.second);
        for (auto &block : availableBlocks) FreeAligned(block.second);
    }
    void *Alloc(size_t nBytes) {
        // Round up _nBytes_ to minimum machine alignment
#if __GNUC__ == 4 && __GNUC_MINOR__ < 9
        // gcc bug: max_align_t wasn't in std:: until 4.9.0
        const int align = alignof(::max_align_t);
#elif !defined(PALADIN_HAVE_ALIGNOF)
        const int align = 16;
#else
        const int align = alignof(std::max_align_t);
#endif
#ifdef PALADIN_HAVE_CONSTEXPR
        static_assert(IsPowerOf2(align), "Minimum alignment not a power of two");
#endif
        nBytes = (nBytes + align - 1) & ~(align - 1);
        if (currentBlockPos + nBytes > currentAllocSize) {
            // Add current block to _usedBlocks_ list
            if (currentBlock) {
                usedBlocks.push_back(
                                     std::make_pair(currentAllocSize, currentBlock));
                currentBlock = nullptr;
                currentAllocSize = 0;
            }
            
            // Get new block of memory for _MemoryArena_
            
            // Try to get memory block from _availableBlocks_
            for (auto iter = availableBlocks.begin();
                 iter != availableBlocks.end(); ++iter) {
                if (iter->first >= nBytes) {
                    currentAllocSize = iter->first;
                    currentBlock = iter->second;
                    availableBlocks.erase(iter);
                    break;
                }
            }
            if (!currentBlock) {
                currentAllocSize = std::max(nBytes, blockSize);
                currentBlock = AllocAligned<uint8_t>(currentAllocSize);
            }
            currentBlockPos = 0;
        }
        void *ret = currentBlock + currentBlockPos;
        currentBlockPos += nBytes;
        return ret;
    }
    template <typename T>
    T *Alloc(size_t n = 1, bool runConstructor = true) {
        T *ret = (T *)Alloc(n * sizeof(T));
        if (runConstructor)
            for (size_t i = 0; i < n; ++i) new (&ret[i]) T();
        return ret;
    }
    void Reset() {
        currentBlockPos = 0;
        availableBlocks.splice(availableBlocks.begin(), usedBlocks);
    }
    size_t TotalAllocated() const {
        size_t total = currentAllocSize;
        for (const auto &alloc : usedBlocks) total += alloc.first;
        for (const auto &alloc : availableBlocks) total += alloc.first;
        return total;
    }
    
private:
    MemoryArena(const MemoryArena &) = delete;
    MemoryArena &operator=(const MemoryArena &) = delete;
    // MemoryArena Private Data
    const size_t blockSize;
    size_t currentBlockPos = 0, currentAllocSize = 0;
    uint8_t *currentBlock = nullptr;
    std::list<std::pair<size_t, uint8_t *>> usedBlocks, availableBlocks;
};

template <typename T, int logBlockSize>
class BlockedArray {
public:
    // BlockedArray Public Methods
    BlockedArray(int uRes, int vRes, const T *d = nullptr)
    : uRes(uRes), vRes(vRes), uBlocks(RoundUp(uRes) >> logBlockSize) {
        int nAlloc = RoundUp(uRes) * RoundUp(vRes);
        data = AllocAligned<T>(nAlloc);
        for (int i = 0; i < nAlloc; ++i) new (&data[i]) T();
        if (d)
            for (int v = 0; v < vRes; ++v)
                for (int u = 0; u < uRes; ++u) (*this)(u, v) = d[v * uRes + u];
    }
    CONSTEXPR int BlockSize() const { return 1 << logBlockSize; }
    int RoundUp(int x) const {
        return (x + BlockSize() - 1) & ~(BlockSize() - 1);
    }
    int uSize() const { return uRes; }
    int vSize() const { return vRes; }
    ~BlockedArray() {
        for (int i = 0; i < uRes * vRes; ++i) data[i].~T();
        FreeAligned(data);
    }
    int Block(int a) const { return a >> logBlockSize; }
    int Offset(int a) const { return (a & (BlockSize() - 1)); }
    T &operator()(int u, int v) {
        int bu = Block(u), bv = Block(v);
        int ou = Offset(u), ov = Offset(v);
        int offset = BlockSize() * BlockSize() * (uBlocks * bv + bu);
        offset += BlockSize() * ov + ou;
        return data[offset];
    }
    const T &operator()(int u, int v) const {
        int bu = Block(u), bv = Block(v);
        int ou = Offset(u), ov = Offset(v);
        int offset = BlockSize() * BlockSize() * (uBlocks * bv + bu);
        offset += BlockSize() * ov + ou;
        return data[offset];
    }
    void GetLinearArray(T *a) const {
        for (int v = 0; v < vRes; ++v)
            for (int u = 0; u < uRes; ++u) *a++ = (*this)(u, v);
    }
    
private:
    // BlockedArray Private Data
    T *data;
    const int uRes, vRes, uBlocks;
};

PALADIN_END

#endif /* memory_hpp */
