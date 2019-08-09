//
//  triangle.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "triangle.hpp"
#include "sampling.hpp"

PALADIN_BEGIN

AABB3f Triangle::objectBound() const {
    const Point3f &p0 = _mesh->points[_vertexIdx[0]];
    const Point3f &p1 = _mesh->points[_vertexIdx[1]];
    const Point3f &p2 = _mesh->points[_vertexIdx[2]];
    AABB3f b1 = AABB3f(worldToObject->exec(p0), worldToObject->exec(p1));
    return unionSet(b1, worldToObject->exec(p2));;
}

bool Triangle::intersect(const Ray &ray, Float *tHit, SurfaceInteraction *isect,
                         bool testAlphaTexture) const {
    // todo
    return true;
}

/*
 基本思路
 三角形的参数方程如下
 u * p0 + v * p1 + (1 - u - v) * p2;
 其中p0，p1和p2是三角形的三个点，u, v是p1和p2的权重，1-u-v是p0的权重，并且满足u>=0, v >= 0,u+v<=1
 ray的方程为o + td
 联合以上两个方程得
 u(p1 - p0) + v(p2 - p0) - td = o - p0
 e1 = p1 - p0
 e2 = p2 - p0
 ue1 + ve2 - td = o - p0
 改写成如下形式
                 t
 [-d  e1  e2] * [u] = T
                 v
*/
bool Triangle::intersectP(const Ray &ray, bool testAlphaTexture) const {
    // todo
    return true;
}

AABB3f Triangle::worldBound() const {
    const Point3f &p0 = _mesh->points[_vertexIdx[0]];
    const Point3f &p1 = _mesh->points[_vertexIdx[1]];
    const Point3f &p2 = _mesh->points[_vertexIdx[2]];
    AABB3f b1 = AABB3f(p0, p1);
    return unionSet(b1, p2);
}

Interaction Triangle::sampleA(const Point2f &u, Float *pdf) const {
    Interaction ret;
    Point2f b = uniformSampleTriangle(u);
    const Point3f &p0 = _mesh->points[_vertexIdx[0]];
    const Point3f &p1 = _mesh->points[_vertexIdx[1]];
    const Point3f &p2 = _mesh->points[_vertexIdx[2]];
    // 2D三角形坐标转换为3D空间三角形坐标
    ret.pos = b[0] * p0 + b[1] * p1 + (1 - b[0] - b[1]) * p2;
    
    ret.normal = normalize(Normal3f(cross(p1 - p0, p2 - p0)));
    
    if (_mesh->normals) {
        Normal3f ns(b[0] * _mesh->normals[_vertexIdx[0]] + b[1] * _mesh->normals[_vertexIdx[1]] +
                    (1 - b[0] - b[1]) * _mesh->normals[_vertexIdx[2]]);
        ret.normal = faceforward(ret.normal, ns);
    } else if (reverseOrientation ^ transformSwapsHandedness) {
        ret.normal *= -1;
    }
    Point3f pAbsSum = abs(b[0] * p0) + abs(b[1] * p1) + abs((1 - b[0] - b[1]) * p2);
    //todo
    ret.pError = gamma(6) * Vector3f(pAbsSum.x, pAbsSum.y, pAbsSum.z);
    *pdf = pdfA();
    return ret;
}

PALADIN_END
