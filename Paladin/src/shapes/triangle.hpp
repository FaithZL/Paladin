//
//  triangle.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef triangle_hpp
#define triangle_hpp

#include "header.h"
#include "shape.hpp"
#include "texture.hpp"
#include <vector>
#include "model.h"

PALADIN_BEGIN

// 三角形网格
struct TriangleMesh {

    TriangleMesh(const Transform &objectToWorld, int nTriangles,
                 const int *vertexIndices, int nVertices, const Point3f *P,
                 const Vector3f *S, const Normal3f *N, const Point2f *uv,
                 const std::shared_ptr<Texture<Float>> &alphaMask,
                 const std::shared_ptr<Texture<Float>> &shadowAlphaMask,
                 const int *faceIndices);
    
    // 三角形个数，顶点个数
    const int nTriangles, nVertices;
    // 顶点的索引
    std::vector<int> vertexIndices;
    // 顶点列表
    std::unique_ptr<Point3f[]> points;
    // 法线列表
    std::unique_ptr<Normal3f[]> normals;
    // 三角形边列表
    std::unique_ptr<Vector3f[]> edges;
    // 参数列表
    std::unique_ptr<Point2f[]> uv;
    std::shared_ptr<Texture<Float>> alphaMask, shadowAlphaMask;
    std::vector<int> faceIndices;
};

class Triangle : public Shape {
public:

    Triangle(const Transform *objectToWorld, const Transform *worldToObject,
             bool reverseOrientation, const std::shared_ptr<TriangleMesh> &_mesh,
             int triNumber)
    : Shape(objectToWorld, worldToObject, reverseOrientation), _mesh(_mesh) {
        v = &_mesh->vertexIndices[3 * triNumber];
        faceIndex = _mesh->faceIndices.size() ? _mesh->faceIndices[triNumber] : 0;
    }

    virtual void init() {
        _invArea = 1 / area();
    }

    virtual AABB3f objectBound() const;

    virtual AABB3f worldBound() const;

    virtual bool intersect(const Ray &ray, Float *tHit, SurfaceInteraction *isect,
                   bool testAlphaTexture = true) const;

    virtual bool intersectP(const Ray &ray, bool testAlphaTexture = true) const;

    virtual Float area() const;
    
    virtual Interaction sampleA(const Point2f &u, Float *pdf) const;
    
    /*
      先把三角形三个顶点投影到单位球上
      球面三角形定理
      球面三角形的三个球面角大小分别为ABC，球面三角形的面积为(A+B+C-π)/r^2
      所以对应的立体角为(A+B+C-π)
     */
    Float solidAngle(const Point3f &p, int nSamples = 0) const {
        std::vector<Vector3f> pSphere = {
            normalize(_mesh->points[v[0]] - p), normalize(_mesh->points[v[1]] - p),
            normalize(_mesh->points[v[2]] - p)
        };
        
        Vector3f cross01 = (cross(pSphere[0], pSphere[1]));
        Vector3f cross12 = (cross(pSphere[1], pSphere[2]));
        Vector3f cross20 = (cross(pSphere[2], pSphere[0]));

        if (cross01.lengthSquared() > 0) {
            cross01 = normalize(cross01);
        }
        if (cross12.lengthSquared() > 0) {
            cross12 = normalize(cross12);
        }
        if (cross20.lengthSquared() > 0) {
            cross20 = normalize(cross20);
        }
        
        return std::abs(
                        std::acos(clamp(dot(cross01, -cross12), -1, 1)) +
                        std::acos(clamp(dot(cross12, -cross20), -1, 1)) +
                        std::acos(clamp(dot(cross20, -cross01), -1, 1)) - Pi);
    }
    
private:

    void getUVs(Point2f uv[3]) const {
        if (_mesh->uv) {
            uv[0] = _mesh->uv[v[0]];
            uv[1] = _mesh->uv[v[1]];
            uv[2] = _mesh->uv[v[2]];
        } else {
            uv[0] = Point2f(0, 0);
            uv[1] = Point2f(1, 0);
            uv[2] = Point2f(1, 1);
        }
    }
    
    std::shared_ptr<TriangleMesh> _mesh;
    const int *v;
    int faceIndex;
};

PALADIN_END

#endif /* triangle_hpp */
