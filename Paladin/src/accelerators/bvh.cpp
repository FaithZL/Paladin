//
//  bvh.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "bvh.hpp"
#include "core/shape.hpp"
#include "shapes/mesh.hpp"

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
    return _nodes ? _nodes[0].bounds : AABB3f();
}

bool BVHAccel::intersect(const paladin::Ray &ray, paladin::SurfaceInteraction *isect) const {
    if (!_nodes) {
        return false;
    }

    bool hit = false;
    Vector3f invDir(1 / ray.dir.x, 1 / ray.dir.y, 1 / ray.dir.z);
    int dirIsNeg[3] = {invDir.x < 0, invDir.y < 0, invDir.z < 0};

    int toVisitOffset = 0, currentNodeIndex = 0;
    int nodesToVisit[64];
    while (true) {
        const LinearBVHNode *node = &_nodes[currentNodeIndex];

        if (node->bounds.intersectP(ray, invDir, dirIsNeg)) {
            if (node->nPrimitives > 0) {
     
                for (int i = 0; i < node->nPrimitives; ++i)
                    if (_primitives[node->primitivesOffset + i]->intersect(ray, isect)) {
                        hit = true;
                    }
                if (toVisitOffset == 0) {
                    break;
                }
                currentNodeIndex = nodesToVisit[--toVisitOffset];
            } else {
                
                if (dirIsNeg[node->axis]) {
                    nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
                    currentNodeIndex = node->secondChildOffset;
                } else {
                    nodesToVisit[toVisitOffset++] = node->secondChildOffset;
                    currentNodeIndex = currentNodeIndex + 1;
                }
            }
        } else {
            if (toVisitOffset == 0) {
                break;
            }
            currentNodeIndex = nodesToVisit[--toVisitOffset];
        }
    }
    return hit;
}

BVHAccel::~BVHAccel() {
    
}

/*
 基本思路
 根据根据光线的方向以及当前节点的分割轴
 选择较近的一个子节点求交，远的子节点放入栈中
 近的子节点如果与光线没有交点，则对栈中的节点求交
 循环以上过程
*/
bool BVHAccel::intersectP(const paladin::Ray &ray) const {
    if (!_nodes) {
        return false;
    }
    Vector3f invDir(1.f / ray.dir.x, 1.f / ray.dir.y, 1.f / ray.dir.z);
    int dirIsNeg[3] = {invDir.x < 0, invDir.y < 0, invDir.z < 0};

    // 即将访问的节点，栈结构
    int nodesToVisit[64];

    // 即将要访问到的节点
    int toVisitOffset = 0;

    // 储存在包围盒在_node数组中节点的位置 
    int currentNodeIndex = 0;
    
    // 从根节点开始遍历
    while (true) {
        const LinearBVHNode *node = &_nodes[currentNodeIndex];
        if (node->bounds.intersectP(ray, invDir, dirIsNeg)) {
            
            if (node->nPrimitives > 0) {
                // 叶子节点
                for (int i = 0; i < node->nPrimitives; ++i) {
                    // 逐个片元判断求交
                    if (_primitives[node->primitivesOffset + i]->intersectP(ray)) {
                        return true;
                    }
                }
                if (toVisitOffset == 0) {
                    break;
                }

                // 取出栈中的节点求交
                currentNodeIndex = nodesToVisit[--toVisitOffset];
            } else {
                // 内部节点
                if (dirIsNeg[node->axis]) {
                    // 如果ray的方向为负，则先判断右子树，把左子树压入栈中
                    // 下次循环时直接判断与右子树是否有相交，如果没有相交
                    // 则访问栈中的节点求交
                    nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
                    currentNodeIndex = node->secondChildOffset;
                } else {
                    // 如果ray的方向为正，则先判断左子树，把右子树压入栈中
                    nodesToVisit[toVisitOffset++] = node->secondChildOffset;
                    currentNodeIndex = currentNodeIndex + 1;
                }
            }
        } else {

            //如果没有相交 则访问栈中的节点求交
            if (toVisitOffset == 0) {
                break;
            }
            currentNodeIndex = nodesToVisit[--toVisitOffset];
        }
    }
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

    // 有序的片元列表
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
    // 将二叉树结构的bvh转换成连续储存结构
    flattenBVHTree(root, &offset);
    CHECK_EQ(totalNodes, offset);
}

BVHAccel::BVHAccel(const vector<shared_ptr<const Shape>> &shapes,
                   int maxPrimsInNode,
                   SplitMethod splitMethod)
: _maxPrimsInNode(std::min(255, maxPrimsInNode)),
_splitMethod(splitMethod),
_shapes(std::move(shapes)) {
    if (_shapes.empty()) {
        return;
    }
    
    
    for (int i = 0; i < _shapes.size(); ++i) {
        auto shape = _shapes[i];
        if (shape->isComplex()) {
            auto mesh = (Mesh *)shape.get();
            const vector<const TriangleI> &triangles = mesh->getTriangles();
            for (int j = 0; j < triangles.size(); ++j) {
                const TriangleI &tri = triangles.at(j);
                _prims.push_back(&tri);
            }
        } else {
            _prims.push_back(shape.get());
        }
    }
    
    std::vector<BVHPrimitiveInfo> _primitiveInfo(_prims.size());
    
    // 储存每个aabb的中心以及索引
    for (size_t i = 0; i < _prims.size(); ++i) {
        auto aabb = _prims[i]->worldBound();
        _primitiveInfo[i] = {i, aabb};
    }
    
    MemoryArena arena(1024 * 1024);
    int totalNodes = 0;
    
    std::vector<const EmbreeUtil::EmbreeGeomtry *> orderedPrims;
    orderedPrims.reserve(_primitives.size());
    BVHBuildNode *root;
    
    if (_splitMethod == SplitMethod::HLBVH) {
        // 暂时不实现
        DCHECK(false);
    } else {
        root = recursiveBuild(arena, _primitiveInfo, 0, _primitiveInfo.size(), &totalNodes, orderedPrims);
    }
    
    _prims.swap(orderedPrims);
    _primitiveInfo.resize(0);
    
    _nodes = allocAligned<LinearBVHNode>(totalNodes);
    int offset = 0;
    // 将二叉树结构的bvh转换成连续储存结构
    flattenBVHTree(root, &offset);
    CHECK_EQ(totalNodes, offset);
}

BVHBuildNode * BVHAccel::recursiveBuild(paladin::MemoryArena &arena, std::vector<BVHPrimitiveInfo> &primitiveInfo, int start, int end, int *totalNodes, std::vector<std::shared_ptr<Primitive> > &orderedPrims) {
    BVHBuildNode *node = ARENA_ALLOC(arena, BVHBuildNode);
    AABB3f bounds;
    for (int i = start; i < end; ++ i) {
        bounds = unionSet(bounds, primitiveInfo[i].bounds);
    }
    (*totalNodes)++;
    int numPrimitives = end - start;
    if (numPrimitives == 1) {
        // 生成叶子节点
        int firstPrimOffset = orderedPrims.size();
        int primNum = primitiveInfo[start].primitiveNumber;
        orderedPrims.push_back(_primitives[primNum]);
        node->initLeaf(firstPrimOffset, numPrimitives, bounds);
        return node;
    } else {
        AABB3f centroidBounds;
        for (int i = start; i < end; ++i) {
            centroidBounds = unionSet(centroidBounds, primitiveInfo[i].centroid);
        }
        // 范围最广的维度
        int maxDim = centroidBounds.maximumExtent();
        int mid = (start + end) / 2;
        // 如果最centroidBounds为一个点，则初始化叶子节点
        if (centroidBounds.pMax[maxDim] == centroidBounds.pMin[maxDim]) {
            int firstPrimOffset = orderedPrims.size();
            for (int i = start; i < end; ++i) {
                int primNum = primitiveInfo[i].primitiveNumber;
                orderedPrims.push_back(_primitives[primNum]);
            }
            node->initLeaf(firstPrimOffset, numPrimitives, bounds);
            return node;
        } else {
            switch (_splitMethod) {
                case Middle: {
                    // 选择范围最大的维度的中点进行划分
                    Float pmid = (centroidBounds.pMin[maxDim] + centroidBounds.pMax[maxDim]) / 2;
                    auto func = [maxDim, pmid](const BVHPrimitiveInfo &pi) {
                        return pi.centroid[maxDim] < pmid;
                    };
                    // std::partition会将区间[first,last)中的元素重新排列，
                    //满足判断条件的元素会被放在区间的前段，不满足的元素会被放在区间的后段。
                    // 返回中间指针
                    BVHPrimitiveInfo *midPtr = std::partition(&primitiveInfo[start], &primitiveInfo[end - 1] + 1,func);
                    mid = midPtr - &primitiveInfo[0];
                    if (mid != start && mid != end) {
                        break;
                    }
                }
                
                case EqualCounts: {
                    // 相等数量划分
                    mid = (start + end) / 2;
                    auto func = [maxDim](const BVHPrimitiveInfo &a,
                                      const BVHPrimitiveInfo &b) {
                        return a.centroid[maxDim] < b.centroid[maxDim];
                    };
                    //将迭代器指向的从_First 到 _last 之间的元素进行二分排序，
                    //以_Nth 为分界，前面都比 _Nth 小（大），后面都比之大（小）；
                    //但是两段内并不有序。
                    std::nth_element(&primitiveInfo[start],
                                     &primitiveInfo[mid],
                                     &primitiveInfo[end - 1] + 1,
                                     func);
                    break;
                }
                    
                case SAH: {
                     // 表面启发式划分
                     // 假设每个片元的求交耗时都相等
                     // 求交消耗的时间为 C = ∑[t=i,N]t(i)
                     // N为片元的数量，t(i)为第i个片元求交耗时
                     // C(A,B) = t1 + p(A) * C(A) + p(B) * C(B)
                     // t1为遍历内部节点所需要的时间加上确定光线通过哪个子节点的时间
                     // p为概率，C为求交耗时
                     // 概率与表面积成正比
                    if (numPrimitives <= 2) {
                        mid = (start + end) / 2;
                        auto func = [maxDim](const BVHPrimitiveInfo &a,
                                          const BVHPrimitiveInfo &b) {
                            return a.centroid[maxDim] < b.centroid[maxDim];
                        };
                        std::nth_element(&primitiveInfo[start],
                                         &primitiveInfo[mid],
                                         &primitiveInfo[end - 1] + 1,
                                         func);
                    } else {
                        // 在范围最广的维度上等距离添加n-1个平面
                        // 把空间分为n个部分，可以理解为n个桶
                        struct BucketInfo {
                            int count = 0;
                            AABB3f bounds;
                        };
                        // 默认12个桶
                        CONSTEXPR int nBuckets = 12;
                        BucketInfo buckets[nBuckets];
                        // 统计每个桶中的bounds以及片元数量
                        for (int i = start; i < end; ++i) {
                            int b = nBuckets * centroidBounds.offset(primitiveInfo[i].centroid)[maxDim];
                            if (b == nBuckets) {
                                b = nBuckets - 1;
                            }
                            buckets[b].count++;
                            buckets[b].bounds = unionSet(buckets[b].bounds, primitiveInfo[i].bounds);
                        }
                        
                        // 找出最优的分割方式，目前假设12个桶，则分割方式有11种
                        // 1与11，2与10，3与9，等等11个组合，估计出每个组合的计算耗时
                        // 从而找出最优的分割方式
                        Float cost[nBuckets - 1];
                        for (int i = 0; i < nBuckets - 1; ++i) {
                            AABB3f b0, b1;
                            int count0 = 0, count1 = 0;
                            // 计算第一部分
                            for (int j = 0; j <= i; ++j) {
                                b0 = unionSet(b0, buckets[j].bounds);
                                count0 += buckets[j].count;
                            }
                            // 计算第二部分
                            for (int j = i + 1; j < nBuckets; ++j) {
                                b1 = unionSet(b1, buckets[j].bounds);
                                count1 += buckets[j].count;
                            }

                            // 参见公式  C(A,B) = t1 + p(A) * C(A) + p(B) * C(B)
                            // p(A) = S(A) / S, p(B) = S(B) / S
                            // 概率与表面积成正比，耗时与片元个数成，
                            // 假设C(A) = count(A)
                            // 则可以写成以下形式

                            // 第一部分的总面积
                            Float s0 = count0 * b0.surfaceArea();
                            // 第二部分的总面积
                            Float s1 = count1 * b1.surfaceArea();
                            // pbrt最新代码把0.125改成了1
                            cost[i] = 1 + (s0 + s1) / bounds.surfaceArea();
                        }
                        
                        // 找到最小耗时的分割方式
                        Float minCost = cost[0];
                        int minCostSplitBucket = 0;
                        for (int i = 1; i < nBuckets - 1; ++i) {
                            if (cost[i] < minCost) {
                                minCost = cost[i];
                                minCostSplitBucket = i;
                            }
                        }
                        // 假设叶子节点的求交耗时等于片元个数
                        Float leafCost = numPrimitives;
                        auto func = [=](const BVHPrimitiveInfo &pi) {
                            int b = nBuckets * centroidBounds.offset(pi.centroid)[maxDim];
                            if (b == nBuckets) {
                                b = nBuckets - 1;
                            }
                            CHECK_GE(b, 0);
                            CHECK_LT(b, nBuckets);
                            return b <= minCostSplitBucket;
                        };

                        
                        if (numPrimitives > _maxPrimsInNode || minCost < leafCost) {
                            // pmid指向第一个func为false的元素的
                            BVHPrimitiveInfo *pmid = std::partition(&primitiveInfo[start],
                                                                &primitiveInfo[end - 1] + 1,
                                                                func);
                            mid = pmid - &primitiveInfo[0];
                        } else {
                            // 创建叶子节点
                            int firstPrimOffset = orderedPrims.size();
                            for (int i = start; i < end; ++i) {
                                int primNum = primitiveInfo[i].primitiveNumber;
                                orderedPrims.push_back(_primitives[primNum]);
                            }
                            node->initLeaf(firstPrimOffset, numPrimitives, bounds);
                            return node;
                        }
                    }
                }
                default:
                    break;
            }
            node->initInterior(maxDim,
                               recursiveBuild(arena, primitiveInfo, start, mid,
                                              totalNodes, orderedPrims),
                               recursiveBuild(arena, primitiveInfo, mid, end,
                                              totalNodes, orderedPrims));
        }
    }
    return node;
}

bool BVHAccel::rayOccluded(const Ray &ray) const {
    if (!_nodes) {
        return false;
    }
    Vector3f invDir(1.f / ray.dir.x, 1.f / ray.dir.y, 1.f / ray.dir.z);
    int dirIsNeg[3] = {invDir.x < 0, invDir.y < 0, invDir.z < 0};

    // 即将访问的节点，栈结构
    int nodesToVisit[64];

    // 即将要访问到的节点
    int toVisitOffset = 0;

    // 储存在包围盒在_node数组中节点的位置
    int currentNodeIndex = 0;
    
    // 从根节点开始遍历
    // 从根节点开始遍历
    while (true) {
        const LinearBVHNode *node = &_nodes[currentNodeIndex];
        if (node->bounds.rayOccluded(ray, invDir, dirIsNeg)) {
            
            if (node->nPrimitives > 0) {
                // 叶子节点
                for (int i = 0; i < node->nPrimitives; ++i) {
                    // 逐个片元判断求交
                    if (_prims[node->primitivesOffset + i]->rayOccluded(ray)) {
                        return true;
                    }
                }
                if (toVisitOffset == 0) {
                    break;
                }

                // 取出栈中的节点求交
                currentNodeIndex = nodesToVisit[--toVisitOffset];
            } else {
                // 内部节点
                if (dirIsNeg[node->axis]) {
                    // 如果ray的方向为负，则先判断右子树，把左子树压入栈中
                    // 下次循环时直接判断与右子树是否有相交，如果没有相交
                    // 则访问栈中的节点求交
                    nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
                    currentNodeIndex = node->secondChildOffset;
                } else {
                    // 如果ray的方向为正，则先判断左子树，把右子树压入栈中
                    nodesToVisit[toVisitOffset++] = node->secondChildOffset;
                    currentNodeIndex = currentNodeIndex + 1;
                }
            }
        } else {

            //如果没有相交 则访问栈中的节点求交
            if (toVisitOffset == 0) {
                break;
            }
            currentNodeIndex = nodesToVisit[--toVisitOffset];
        }
    }
    return false;
}

BVHBuildNode * BVHAccel::recursiveBuild(MemoryArena &arena, std::vector<BVHPrimitiveInfo> &primitiveInfo, int start, int end, int *totalNodes, std::vector<const EmbreeUtil::EmbreeGeomtry *> &orderedPrims) {
    BVHBuildNode *node = ARENA_ALLOC(arena, BVHBuildNode);
    AABB3f bounds;
    for (int i = start; i < end; ++ i) {
        bounds = unionSet(bounds, primitiveInfo[i].bounds);
    }
    (*totalNodes)++;
    int numPrimitives = end - start;
    if (numPrimitives == 1) {
        // 生成叶子节点
        int firstPrimOffset = orderedPrims.size();
        int primNum = primitiveInfo[start].primitiveNumber;
        orderedPrims.push_back(_prims[primNum]);
        node->initLeaf(firstPrimOffset, numPrimitives, bounds);
        return node;
    } else {
        AABB3f centroidBounds;
        for (int i = start; i < end; ++i) {
            centroidBounds = unionSet(centroidBounds, primitiveInfo[i].centroid);
        }
        // 范围最广的维度
        int maxDim = centroidBounds.maximumExtent();
        int mid = (start + end) / 2;
        // 如果最centroidBounds为一个点，则初始化叶子节点
        if (centroidBounds.pMax[maxDim] == centroidBounds.pMin[maxDim]) {
            int firstPrimOffset = orderedPrims.size();
            for (int i = start; i < end; ++i) {
                int primNum = primitiveInfo[i].primitiveNumber;
                orderedPrims.push_back(_prims[primNum]);
            }
            node->initLeaf(firstPrimOffset, numPrimitives, bounds);
            return node;
        } else {
            switch (_splitMethod) {
                case Middle: {
                    // 选择范围最大的维度的中点进行划分
                    Float pmid = (centroidBounds.pMin[maxDim] + centroidBounds.pMax[maxDim]) / 2;
                    auto func = [maxDim, pmid](const BVHPrimitiveInfo &pi) {
                        return pi.centroid[maxDim] < pmid;
                    };
                    // std::partition会将区间[first,last)中的元素重新排列，
                    //满足判断条件的元素会被放在区间的前段，不满足的元素会被放在区间的后段。
                    // 返回中间指针
                    BVHPrimitiveInfo *midPtr = std::partition(&primitiveInfo[start], &primitiveInfo[end - 1] + 1,func);
                    mid = midPtr - &primitiveInfo[0];
                    if (mid != start && mid != end) {
                        break;
                    }
                }
                
                case EqualCounts: {
                    // 相等数量划分
                    mid = (start + end) / 2;
                    auto func = [maxDim](const BVHPrimitiveInfo &a,
                                      const BVHPrimitiveInfo &b) {
                        return a.centroid[maxDim] < b.centroid[maxDim];
                    };
                    //将迭代器指向的从_First 到 _last 之间的元素进行二分排序，
                    //以_Nth 为分界，前面都比 _Nth 小（大），后面都比之大（小）；
                    //但是两段内并不有序。
                    std::nth_element(&primitiveInfo[start],
                                     &primitiveInfo[mid],
                                     &primitiveInfo[end - 1] + 1,
                                     func);
                    break;
                }
                    
                case SAH: {
                     // 表面启发式划分
                     // 假设每个片元的求交耗时都相等
                     // 求交消耗的时间为 C = ∑[t=i,N]t(i)
                     // N为片元的数量，t(i)为第i个片元求交耗时
                     // C(A,B) = t1 + p(A) * C(A) + p(B) * C(B)
                     // t1为遍历内部节点所需要的时间加上确定光线通过哪个子节点的时间
                     // p为概率，C为求交耗时
                     // 概率与表面积成正比
                    if (numPrimitives <= 2) {
                        mid = (start + end) / 2;
                        auto func = [maxDim](const BVHPrimitiveInfo &a,
                                          const BVHPrimitiveInfo &b) {
                            return a.centroid[maxDim] < b.centroid[maxDim];
                        };
                        std::nth_element(&primitiveInfo[start],
                                         &primitiveInfo[mid],
                                         &primitiveInfo[end - 1] + 1,
                                         func);
                    } else {
                        // 在范围最广的维度上等距离添加n-1个平面
                        // 把空间分为n个部分，可以理解为n个桶
                        struct BucketInfo {
                            int count = 0;
                            AABB3f bounds;
                        };
                        // 默认12个桶
                        CONSTEXPR int nBuckets = 12;
                        BucketInfo buckets[nBuckets];
                        // 统计每个桶中的bounds以及片元数量
                        for (int i = start; i < end; ++i) {
                            int b = nBuckets * centroidBounds.offset(primitiveInfo[i].centroid)[maxDim];
                            if (b == nBuckets) {
                                b = nBuckets - 1;
                            }
                            buckets[b].count++;
                            buckets[b].bounds = unionSet(buckets[b].bounds, primitiveInfo[i].bounds);
                        }
                        
                        // 找出最优的分割方式，目前假设12个桶，则分割方式有11种
                        // 1与11，2与10，3与9，等等11个组合，估计出每个组合的计算耗时
                        // 从而找出最优的分割方式
                        Float cost[nBuckets - 1];
                        for (int i = 0; i < nBuckets - 1; ++i) {
                            AABB3f b0, b1;
                            int count0 = 0, count1 = 0;
                            // 计算第一部分
                            for (int j = 0; j <= i; ++j) {
                                b0 = unionSet(b0, buckets[j].bounds);
                                count0 += buckets[j].count;
                            }
                            // 计算第二部分
                            for (int j = i + 1; j < nBuckets; ++j) {
                                b1 = unionSet(b1, buckets[j].bounds);
                                count1 += buckets[j].count;
                            }

                            // 参见公式  C(A,B) = t1 + p(A) * C(A) + p(B) * C(B)
                            // p(A) = S(A) / S, p(B) = S(B) / S
                            // 概率与表面积成正比，耗时与片元个数成，
                            // 假设C(A) = count(A)
                            // 则可以写成以下形式

                            // 第一部分的总面积
                            Float s0 = count0 * b0.surfaceArea();
                            // 第二部分的总面积
                            Float s1 = count1 * b1.surfaceArea();
                            // pbrt最新代码把0.125改成了1
                            cost[i] = 1 + (s0 + s1) / bounds.surfaceArea();
                        }
                        
                        // 找到最小耗时的分割方式
                        Float minCost = cost[0];
                        int minCostSplitBucket = 0;
                        for (int i = 1; i < nBuckets - 1; ++i) {
                            if (cost[i] < minCost) {
                                minCost = cost[i];
                                minCostSplitBucket = i;
                            }
                        }
                        // 假设叶子节点的求交耗时等于片元个数
                        Float leafCost = numPrimitives;
                        auto func = [=](const BVHPrimitiveInfo &pi) {
                            int b = nBuckets * centroidBounds.offset(pi.centroid)[maxDim];
                            if (b == nBuckets) {
                                b = nBuckets - 1;
                            }
                            CHECK_GE(b, 0);
                            CHECK_LT(b, nBuckets);
                            return b <= minCostSplitBucket;
                        };

                        
                        if (numPrimitives > _maxPrimsInNode || minCost < leafCost) {
                            // pmid指向第一个func为false的元素的
                            BVHPrimitiveInfo *pmid = std::partition(&primitiveInfo[start],
                                                                &primitiveInfo[end - 1] + 1,
                                                                func);
                            mid = pmid - &primitiveInfo[0];
                        } else {
                            // 创建叶子节点
                            int firstPrimOffset = orderedPrims.size();
                            for (int i = start; i < end; ++i) {
                                int primNum = primitiveInfo[i].primitiveNumber;
                                orderedPrims.push_back(_prims[primNum]);
                            }
                            node->initLeaf(firstPrimOffset, numPrimitives, bounds);
                            return node;
                        }
                    }
                }
                default:
                    break;
            }
            node->initInterior(maxDim,
                               recursiveBuild(arena, primitiveInfo, start, mid,
                                              totalNodes, orderedPrims),
                               recursiveBuild(arena, primitiveInfo, mid, end,
                                              totalNodes, orderedPrims));
        }
    }
    return node;
}

bool BVHAccel::rayIntersect(const Ray &ray, SurfaceInteraction *isect) const {
    if (!_nodes) {
        return false;
    }

    bool hit = false;
    Vector3f invDir(1 / ray.dir.x, 1 / ray.dir.y, 1 / ray.dir.z);
    int dirIsNeg[3] = {invDir.x < 0, invDir.y < 0, invDir.z < 0};

    int toVisitOffset = 0, currentNodeIndex = 0;
    int nodesToVisit[64];
    while (true) {
        const LinearBVHNode *node = &_nodes[currentNodeIndex];

        if (node->bounds.rayOccluded(ray, invDir, dirIsNeg)) {
            if (node->nPrimitives > 0) {
                // 叶子节点
                for (int i = 0; i < node->nPrimitives; ++i) {
                    // 逐个片元判断求交
                    if (_prims[node->primitivesOffset + i]->rayIntersect(ray, isect)) {
                        hit = true;
                    }
                }
                if (toVisitOffset == 0) {
                    break;
                }
                // 取出栈中的节点求交
                currentNodeIndex = nodesToVisit[--toVisitOffset];
                if (currentNodeIndex == 1) {
                    
                }
            } else {
                // 内部节点
                if (dirIsNeg[node->axis]) {
                    // 如果ray的方向为负，则先判断右子树，把左子树压入栈中
                    // 下次循环时直接判断与右子树是否有相交，如果没有相交
                    // 则访问栈中的节点求交
                    nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
                    currentNodeIndex = node->secondChildOffset;
                } else {
                    // 如果ray的方向为正，则先判断左子树，把右子树压入栈中
                    nodesToVisit[toVisitOffset++] = node->secondChildOffset;
                    currentNodeIndex = currentNodeIndex + 1;
                }
            }
        } else {
            //如果没有相交 则访问栈中的节点求交
            if (toVisitOffset == 0) {
                break;
            }
            currentNodeIndex = nodesToVisit[--toVisitOffset];
        }
    }
    return hit;
}

BVHBuildNode * BVHAccel::HLBVHBuild(paladin::MemoryArena &arena, const std::vector<BVHPrimitiveInfo> &primitiveInfo, int *totalNodes, std::vector<std::shared_ptr<Primitive> > &orderedPrims) const {
    // HLBVH构建相关 todo
    return nullptr;
}

BVHBuildNode * BVHAccel::buildUpperSAH(paladin::MemoryArena &arena, std::vector<BVHBuildNode *> &treeletRoots, int start, int end, int *totalNodes) const {
    // HLBVH构建相关 todo
    return nullptr;
}

BVHBuildNode * BVHAccel::emitLBVH(paladin::BVHBuildNode *&buildNodes, const std::vector<BVHPrimitiveInfo> &primitiveInfo, paladin::MortonPrimitive *mortonPrims, int nPrimitives, int *totalNodes, std::vector<std::shared_ptr<Primitive> > &orderedPrims, std::atomic<int> *orderedPrimsOffset, int bitIndex) const {
    // HLBVH构建相关 todo
    return nullptr;
}

int BVHAccel::flattenBVHTree(paladin::BVHBuildNode *node, int *offset) {
    LinearBVHNode *linearNode = &_nodes[*offset];
    linearNode->bounds = node->bounds;
    int myOffset = (*offset)++;
    if (node->nPrimitives > 0) {
        // 初始化叶子节点
        DCHECK(!node->children[0] && !node->children[1]);
        CHECK_LT(node->nPrimitives, 65536);
        linearNode->primitivesOffset = node->firstPrimOffset;
        linearNode->nPrimitives = node->nPrimitives;
    } else {
        // 初始化内部节点
        linearNode->axis = node->splitAxis;
        linearNode->nPrimitives = 0;
        flattenBVHTree(node->children[0], offset);
        linearNode->secondChildOffset =
            flattenBVHTree(node->children[1], offset);
    }
    return myOffset;
}

//"param" : {
//    "maxPrimsInNode" : 1,
//    "splitMethod" : "SAH"
//}
shared_ptr<BVHAccel> createBVH(const nloJson &param, const vector<shared_ptr<Primitive>> &prims) {
    int maxPrimsInNode = param.value("maxPrimsInNode", 1);
    BVHAccel::SplitMethod splitMethod;
    string sm = param.value("splitMethod", "SAH");
    if (sm == "SAH") {
        splitMethod = BVHAccel::SplitMethod::SAH;
    } else if (sm == "Middle") {
        splitMethod = BVHAccel::SplitMethod::Middle;
    } else if (sm == "EqualCounts") {
        splitMethod = BVHAccel::SplitMethod::EqualCounts;
    }
    return make_shared<BVHAccel>(prims, maxPrimsInNode, splitMethod);
}

//"param" : {
//    "maxPrimsInNode" : 1,
//    "splitMethod" : "SAH"
//}
shared_ptr<BVHAccel> createBVH(const nloJson &param, const vector<shared_ptr<const Shape>> &shapes) {
    int maxPrimsInNode = param.value("maxPrimsInNode", 1);
    BVHAccel::SplitMethod splitMethod;
    string sm = param.value("splitMethod", "SAH");
    if (sm == "SAH") {
        splitMethod = BVHAccel::SplitMethod::SAH;
    } else if (sm == "Middle") {
        splitMethod = BVHAccel::SplitMethod::Middle;
    } else if (sm == "EqualCounts") {
        splitMethod = BVHAccel::SplitMethod::EqualCounts;
    }
    return make_shared<BVHAccel>(shapes, maxPrimsInNode, splitMethod);
}

PALADIN_END
