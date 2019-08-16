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

AABB3f BVHAccel::worldBound() const {
    return AABB3f();
}

bool BVHAccel::intersect(const paladin::Ray &ray, paladin::SurfaceInteraction *isect) const {
    return false;
}

BVHAccel::~BVHAccel() {
    
}

bool BVHAccel::intersectP(const paladin::Ray &ray) const {
    return false;
}

BVHAccel::BVHAccel(std::vector<std::shared_ptr<Primitive>> p,
                   int maxPrimsInNode, SplitMethod splitMethod)
: _maxPrimsInNode(std::min(255, maxPrimsInNode)),
_splitMethod(splitMethod),
_primitives(std::move(p)) {

    // 基本思路，先构建出树形结构
    // 完成之后再把树形结构转成连续储存
    if (_primitives.empty()) {
        return;
    }
    
    std::vector<BVHPrimitiveInfo> _primitiveInfo(_primitives.size());
    // 储存每个aabb的中心以及索引
    for (size_t i = 0; i < _primitives.size(); ++i) {
        _primitiveInfo[i] = {i, _primitives[i]->worldBound()};
    }

    // 先使用内存池分配指定大小空间，函数运行结束之后自动释放
    MemoryArena arena(1024 * 1024);
    int totalNodes = 0;
    std::vector<std::shared_ptr<Primitive>> orderedPrims;
    orderedPrims.reserve(_primitives.size());
    BVHBuildNode *root;
    if (splitMethod == SplitMethod::HLBVH) {
        // HLBVH可以用并行构建
        root = HLBVHBuild(arena, _primitiveInfo, &totalNodes, orderedPrims);
    } else {
        // 其余三种方式
        root = recursiveBuild(arena, _primitiveInfo, 0, _primitives.size(),
                              &totalNodes, orderedPrims);
    }
    _primitives.swap(orderedPrims);
    _primitiveInfo.resize(0);
    
    
    _nodes = allocAligned<LinearBVHNode>(totalNodes);
    int offset = 0;
    // 转换成连续储存
    flattenBVHTree(root, &offset);
    CHECK_EQ(totalNodes, offset);
}

BVHBuildNode * BVHAccel::recursiveBuild(paladin::MemoryArena &arena, std::vector<BVHPrimitiveInfo> &primitiveInfo, int start, int end, int *totalNodes, std::vector<std::shared_ptr<Primitive> > &orderedPrims) {
    BVHBuildNode *node = ARENA_ALLOC(arena, BVHBuildNode);
    AABB3f bounds;
    for (int i = start; i < end; ++ i) {
        bounds = unionSet(bounds, primitiveInfo[i].bounds);
    }
    
    int numPrimitives = end - start;
    if (numPrimitives == 1) {
        // 生成叶子节点
        int firstPrimOffset = orderedPrims.size();
        for (int i = start; i < end; ++i) {
            int primNum = primitiveInfo[i].primitiveNumber;
            orderedPrims.push_back(_primitives[primNum]);
        }
        node->initLeaf(firstPrimOffset, numPrimitives, bounds);
        return node;
    } else {
        AABB3f centroidBounds;
    }
}

BVHBuildNode * BVHAccel::HLBVHBuild(paladin::MemoryArena &arena, const std::vector<BVHPrimitiveInfo> &primitiveInfo, int *totalNodes, std::vector<std::shared_ptr<Primitive> > &orderedPrims) const {
    
    return nullptr;
}

BVHBuildNode * BVHAccel::buildUpperSAH(paladin::MemoryArena &arena, std::vector<BVHBuildNode *> &treeletRoots, int start, int end, int *totalNodes) const {
    
    return nullptr;
}

BVHBuildNode * BVHAccel::emitLBVH(paladin::BVHBuildNode *&buildNodes, const std::vector<BVHPrimitiveInfo> &primitiveInfo, paladin::MortonPrimitive *mortonPrims, int nPrimitives, int *totalNodes, std::vector<std::shared_ptr<Primitive> > &orderedPrims, std::atomic<int> *orderedPrimsOffset, int bitIndex) const {
    
    return nullptr;
}

int BVHAccel::flattenBVHTree(paladin::BVHBuildNode *node, int *offset) {
    return 0;
}

PALADIN_END
