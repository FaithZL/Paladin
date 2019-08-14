//
//  bvh.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef bvh_hpp
#define bvh_hpp

#include "header.h"
#include "primitive.hpp"
PALADIN_BEGIN

//代表一个图元的部分信息
struct BVHPrimitiveInfo {
    BVHPrimitiveInfo() {}
    BVHPrimitiveInfo(size_t primitiveNumber, const AABB3f &bounds)
    : primitiveNumber(primitiveNumber),
    bounds(bounds),
    centroid(.5f * bounds.pMin + .5f * bounds.pMax) {}
    size_t primitiveNumber;
    AABB3f bounds;
    Point3f centroid;
};

// bvh可以理解为一个二叉树
struct BVHBuildNode {
    void InitLeaf(int first, int n, const AABB3f &b) {
        firstPrimOffset = first;
        nPrimitives = n;
        bounds = b;
        children[0] = children[1] = nullptr;
    }
    void InitInterior(int axis, BVHBuildNode *c0, BVHBuildNode *c1) {
        children[0] = c0;
        children[1] = c1;
        bounds = unionSet(c0->bounds, c1->bounds);
        splitAxis = axis;
        nPrimitives = 0;
    }
    AABB3f bounds;
    // 二叉树的两个子节点
    BVHBuildNode *children[2];
    // 分割的坐标轴
    int splitAxis;
    // 第一个片元的偏移量
    int firstPrimOffset;
    // 片元数量
    int nPrimitives;
};

// 莫顿码片元
struct MortonPrimitive {
    int primitiveIndex;
    uint32_t mortonCode;
};

struct LBVHTreelet {
    int startIndex, nPrimitives;
    BVHBuildNode *buildNodes;
};

//在内存中的一个线性BVH节点
struct LinearBVHNode {
    AABB3f bounds;
    union {
        int primitivesOffset;   //指向图元
        int secondChildOffset;  // interior
    };
    uint16_t nPrimitives;  // 0 -> interior node
    uint8_t axis;          // interior node: xyz
    uint8_t pad[1];        // 确保32个字节为一个对象，提高缓存命中率
};


/*
 根据对象划分
 */
class BVHAccel : public Aggregate {
    
    enum class SplitMethod { SAH, HLBVH, Middle, EqualCounts };
    
    BVHAccel(std::vector<std::shared_ptr<Primitive>> p,
             int maxPrimsInNode = 1,
             SplitMethod splitMethod = SplitMethod::SAH);
    
    virtual AABB3f worldBound() const;
    
    ~BVHAccel();
    virtual bool intersect(const Ray &ray, SurfaceInteraction *isect) const;
    virtual bool ntersectP(const Ray &ray) const;
    
private:
    BVHBuildNode *recursiveBuild(
                                 MemoryArena &arena, std::vector<BVHPrimitiveInfo> &primitiveInfo,
                                 int start, int end, int *totalNodes,
                                 std::vector<std::shared_ptr<Primitive>> &orderedPrims);
    
    BVHBuildNode *HLBVHBuild(
                             MemoryArena &arena, const std::vector<BVHPrimitiveInfo> &primitiveInfo,
                             int *totalNodes,
                             std::vector<std::shared_ptr<Primitive>> &orderedPrims) const;
    
    BVHBuildNode *emitLBVH(
                           BVHBuildNode *&buildNodes,
                           const std::vector<BVHPrimitiveInfo> &primitiveInfo,
                           MortonPrimitive *mortonPrims, int nPrimitives, int *totalNodes,
                           std::vector<std::shared_ptr<Primitive>> &orderedPrims,
                           std::atomic<int> *orderedPrimsOffset, int bitIndex) const;
    
    BVHBuildNode *buildUpperSAH(MemoryArena &arena,
                                std::vector<BVHBuildNode *> &treeletRoots,
                                int start, int end, int *totalNodes) const;
    
    int flattenBVHTree(BVHBuildNode *node, int *offset);
    
    const int _maxPrimsInNode;
    const SplitMethod _splitMethod;
    std::vector<std::shared_ptr<Primitive>> _primitives;
    LinearBVHNode *_nodes = nullptr;
};

PALADIN_END

#endif /* bvh_hpp */
