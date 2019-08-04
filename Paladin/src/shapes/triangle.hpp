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
    

    const int nTriangles, nVertices;
    std::vector<int> vertexIndices;
    std::unique_ptr<Point3f[]> points;
    std::unique_ptr<Normal3f[]> normals;
    std::unique_ptr<Vector3f[]> edges;
    std::unique_ptr<Point2f[]> uv;
    std::shared_ptr<Texture<Float>> alphaMask, shadowAlphaMask;
    std::vector<int> faceIndices;
};

class Triangle : public Shape {
public:

    Triangle(const Transform *objectToWorld, const Transform *worldToObject,
             bool reverseOrientation, const std::shared_ptr<TriangleMesh> &mesh,
             int triNumber)
    : Shape(objectToWorld, worldToObject, reverseOrientation), mesh(mesh) {
        v = &mesh->vertexIndices[3 * triNumber];
        faceIndex = mesh->faceIndices.size() ? mesh->faceIndices[triNumber] : 0;
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
    
    using Shape::sampleA;
    virtual Interaction sampleA(const Point2f &u, Float *pdf) const;
    

    // Float SolidAngle(const Point3f &p, int nSamples = 0) const;
    
private:

    void getUVs(Point2f uv[3]) const {
        if (mesh->uv) {
            uv[0] = mesh->uv[v[0]];
            uv[1] = mesh->uv[v[1]];
            uv[2] = mesh->uv[v[2]];
        } else {
            uv[0] = Point2f(0, 0);
            uv[1] = Point2f(1, 0);
            uv[2] = Point2f(1, 1);
        }
    }
    
    std::shared_ptr<TriangleMesh> mesh;
    const int *v;
    int faceIndex;
};

PALADIN_END

#endif /* triangle_hpp */
