//
//  transformcache.h
//  Paladin
//
//  Created by SATAN_Z on 2019/11/11.
//

#ifndef transformcache_h
#define transformcache_h

#include "core/header.h"
#include "tools/memory.hpp"
#include "math/transform.hpp"

PALADIN_BEGIN

STAT_MEMORY_COUNTER("Memory/TransformCache", transformCacheBytes);
STAT_PERCENT("Scene/TransformCache hits", nTransformCacheHits, nTransformCacheLookups);
STAT_INT_DISTRIBUTION("Scene/Probes per TransformCache lookup", transformCacheProbes);

// 直接照搬了pbrt代码，搞完主线再回头搞搞
class TransformCache {
public:
    TransformCache()
    : hashTable(512),
    hashTableOccupancy(0) {
        
    }

    Transform *Lookup(const Transform &t) {
        ++nTransformCacheLookups;
        int offset = Hash(t) & (hashTable.size() - 1);
        int step = 1;
        while (true) {

            if (!hashTable[offset] || *hashTable[offset] == t) {
                break;
            }
            offset = (offset + step * step) & (hashTable.size() - 1);
            ++step;
        }
        Transform *tCached = hashTable[offset];
        if (tCached) {
            ++nTransformCacheHits;
        } else {
            tCached = arena.alloc<Transform>();
            *tCached = t;
            Insert(tCached);
        }
        return tCached;
    }

    void Clear() {
        transformCacheBytes += arena.totalAllocated() + hashTable.size() * sizeof(Transform *);
        hashTable.clear();
        hashTable.resize(512);
        hashTableOccupancy = 0;
        arena.reset();
    }

private:
    void Insert(Transform *tNew) {
        if (++hashTableOccupancy == hashTable.size() / 2) {
            Grow();
        }

        int offset = Hash(*tNew) & (hashTable.size() - 1);
        int step = 1;
        while (true) {
            if (hashTable[offset] == nullptr) {
                hashTable[offset] = tNew;
                return;
            }
            offset = (offset + step * step) & (hashTable.size() - 1);
            ++step;
        }
    }
    void Grow() {
        std::vector<Transform *> newTable(2 * hashTable.size());
        LOG(INFO) << "Growing transform cache hash table to " << newTable.size();

        for (Transform *tEntry : hashTable) {
            if (!tEntry) continue;

            int offset = Hash(*tEntry) & (newTable.size() - 1);
            int step = 1;
            while (true) {
                if (newTable[offset] == nullptr) {
                    newTable[offset] = tEntry;
                    break;
                }
                offset = (offset + step * step) & (hashTable.size() - 1);
                ++step;
            }
        }

        std::swap(hashTable, newTable);
    }

    static uint64_t Hash(const Transform &t) {
        const char *ptr = (const char *)(&t.getMatrix());
        size_t size = sizeof(Matrix4x4);
        uint64_t hash = 14695981039346656037ull;
        while (size > 0) {
            hash ^= *ptr;
            hash *= 1099511628211ull;
            ++ptr;
            --size;
        }
        return hash;
    }

    std::vector<Transform *> hashTable;
    int hashTableOccupancy;
    MemoryArena arena;
};


PALADIN_END

#endif /* transformcache_h */
