//
//  bvh.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "bvh.hpp"

PALADIN_BEGIN

// BVHAccel Utility Functions
inline uint32_t LeftShift3(uint32_t x) {
    CHECK_LE(x, (1 << 10));
    if (x == (1 << 10)) --x;
#ifdef PALADIN_HAVE_BINARY_CONSTANTS
    x = (x | (x << 16)) & 0b00000011000000000000000011111111;
    // x = ---- --98 ---- ---- ---- ---- 7654 3210
    x = (x | (x << 8)) & 0b00000011000000001111000000001111;
    // x = ---- --98 ---- ---- 7654 ---- ---- 3210
    x = (x | (x << 4)) & 0b00000011000011000011000011000011;
    // x = ---- --98 ---- 76-- --54 ---- 32-- --10
    x = (x | (x << 2)) & 0b00001001001001001001001001001001;
    // x = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
#else
    x = (x | (x << 16)) & 0x30000ff;
    // x = ---- --98 ---- ---- ---- ---- 7654 3210
    x = (x | (x << 8)) & 0x300f00f;
    // x = ---- --98 ---- ---- 7654 ---- ---- 3210
    x = (x | (x << 4)) & 0x30c30c3;
    // x = ---- --98 ---- 76-- --54 ---- 32-- --10
    x = (x | (x << 2)) & 0x9249249;
    // x = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
#endif // PBRT_HAVE_BINARY_CONSTANTS
    return x;
}

BVHAccel::BVHAccel(std::vector<std::shared_ptr<Primitive>> p,
                   int maxPrimsInNode, SplitMethod splitMethod)
: _maxPrimsInNode(std::min(255, maxPrimsInNode)),
_splitMethod(splitMethod),
_primitives(std::move(p)) {

    if (_primitives.empty()) {
        return;
    }
    
    std::vector<BVHPrimitiveInfo> _primitiveInfo(_primitives.size());
    
    for (size_t i = 0; i < _primitives.size(); ++i) {
        _primitiveInfo[i] = {i, _primitives[i]->worldBound()};
    }
    
    MemoryArena arena(1024 * 1024);
    int totalNodes = 0;
    std::vector<std::shared_ptr<Primitive>> orderedPrims;
    orderedPrims.reserve(_primitives.size());
    BVHBuildNode *root;
    if (splitMethod == SplitMethod::HLBVH) {
        root = HLBVHBuild(arena, _primitiveInfo, &totalNodes, orderedPrims);
    } else {
        root = recursiveBuild(arena, _primitiveInfo, 0, _primitives.size(),
                              &totalNodes, orderedPrims);
    }
    _primitives.swap(orderedPrims);
    _primitiveInfo.resize(0);
    COUT << StringPrintf("BVH created with %d nodes for %d "
                              "primitives (%.2f MB), arena allocated %.2f MB",
                              totalNodes, (int)_primitives.size(),
                              float(totalNodes * sizeof(LinearBVHNode)) /
                              (1024.f * 1024.f),
                              float(arena.totalAllocated()) /
                              (1024.f * 1024.f));
    
    
    _nodes = allocAligned<LinearBVHNode>(totalNodes);
    int offset = 0;
    flattenBVHTree(root, &offset);
    CHECK_EQ(totalNodes, offset);
}

BVHBuildNode * BVHAccel::recursiveBuild(paladin::MemoryArena &arena, std::vector<BVHPrimitiveInfo> &primitiveInfo, int start, int end, int *totalNodes, std::vector<std::shared_ptr<Primitive> > &orderedPrims) {
    return NULL;
    
}

PALADIN_END
