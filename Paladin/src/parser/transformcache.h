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

// 直接照搬了pbrt代码，搞完主线再回头搞搞
class TransformCache {
public:
    TransformCache()
    : hashTable(512),
    hashTableOccupancy(0) {
        
    }

    Transform *Lookup(const Transform &t) {

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
            
        } else {
            tCached = arena.alloc<Transform>();
            *tCached = t;
            Insert(tCached);
        }
        return tCached;
    }

    void Clear() {
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

inline void testCache() {
    TransformCache tc;
    
    auto t1 = Transform::scale(1, 2, 1);
    auto t2 = Transform::scale(1, 2, 1);
    Transform * t3 = tc.Lookup(t1);
    tc.Lookup(t2);
    COUT << *t3 << std::endl;
}

PALADIN_END

#endif /* transformcache_h */