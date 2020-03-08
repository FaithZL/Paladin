//
//  triangle.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef trianglemesh_hpp
#define trianglemesh_hpp

#include "core/header.h"
#include "core/shape.hpp"
#include "core/texture.hpp"
#include <vector>

PALADIN_BEGIN

class Triangle;

struct Index {
    Index(int uv, int pos, int normal, int edge):
    uv(uv),
    pos(pos),
    normal(normal),
    edge(edge) {
        
    }
    
    Index(int idx)
    : uv(idx),
    pos(idx),
    normal(idx),
    edge(idx) {
        
    }
    
    Index() {
        uv = 0;
        pos = 0;
        normal = 0;
        edge = 0;
    }
    
    // 顶点uv索引
    int uv;
    // 顶点位置索引
    int pos;
    // 顶点法线索引
    int normal;
    //
    int edge;
};

/*
 三角形网格，所以的向量法线以及点都是在世界坐标上
 */
struct TriangleMesh {

    TriangleMesh(const shared_ptr<const Transform> &objectToWorld, int nTriangles,
                 const int *vertexIndices, int nVertices, const Point3f *P,
                 const Vector3f *S, const Normal3f *N, const Point2f *uv,
                 const std::shared_ptr<Texture<Float>> &alphaMask=nullptr,
                 const std::shared_ptr<Texture<Float>> &shadowAlphaMask=nullptr,
                 const int *faceIndices=nullptr);
    
    TriangleMesh(const shared_ptr<const Transform> &objectToWorld, int nTriangles,
                 const vector<Index> &vertexIndices, const vector<Point3f> *P,
                 const vector<Normal3f> *N=nullptr, const vector<Point2f> *UV=nullptr, const vector<Vector3f> *E=nullptr,
                 const std::shared_ptr<Texture<Float>> &alphaMask=nullptr,
                 const std::shared_ptr<Texture<Float>> &shadowAlphaMask=nullptr,
                 const int *faceIndices=nullptr);
    
    static std::shared_ptr<TriangleMesh> create(const shared_ptr<const Transform> &objectToWorld, int nTriangles,
                                        const vector<Index> &vertexIndices, const vector<Point3f> *P,
                                        const vector<Normal3f> *N=nullptr, const vector<Point2f> *UV=nullptr, const vector<Vector3f> *E=nullptr,
                                        const std::shared_ptr<Texture<Float>> &alphaMask=nullptr,
                                        const std::shared_ptr<Texture<Float>> &shadowAlphaMask=nullptr,
                                        const int *faceIndices=nullptr);


private:
    // 三角形个数，顶点个数
    const int nTriangles, nVertices;
    
    vector<Index> vertexIndice;
    
    // 法线索引
    std::vector<int> normalIndices;
    // 纹理索引
    std::vector<int> uvIndices;
    // 顶点列表，为世界坐标
    std::unique_ptr<Point3f[]> points;
    // 法线列表
    std::unique_ptr<Normal3f[]> normals;
    // 三角形边列表
    std::unique_ptr<Vector3f[]> edges;
    // 参数列表
    std::unique_ptr<Point2f[]> uv;
    std::shared_ptr<Texture<Float>> alphaMask, shadowAlphaMask;
    std::vector<int> faceIndices;
    
    friend class Triangle;
};

class Triangle : public Shape {
public:

    Triangle(const shared_ptr<const Transform> &objectToWorld, const shared_ptr<const Transform> &worldToObject,
             bool reverseOrientation, const std::shared_ptr<TriangleMesh> &_mesh,
             int triNumber)
    : Shape(objectToWorld, worldToObject, reverseOrientation), _mesh(_mesh) {
        _vertexIdx = &_mesh->vertexIndice[3 * triNumber];
        
        _faceIndex = _mesh->faceIndices.size() ? _mesh->faceIndices[triNumber] : 0;
    }

    virtual nloJson toJson() const override {
        return nloJson();
    }
    
    virtual void init() override {
        _invArea = 1 / area();
    }

    virtual AABB3f objectBound() const override;

    virtual AABB3f worldBound() const override;

    bool watertightIntersectP(const Ray &ray, bool testAlphaTexture) const;
    
    bool watertightIntersect(const Ray &ray, Float *tHit, SurfaceInteraction *isect,
                             bool testAlphaTexture = true) const;
    
    bool classicIntersectP(const Ray &ray, bool testAlphaTexture) const;
    
    bool classicIntersect(const Ray &ray, Float *tHit, SurfaceInteraction *isect,
                          bool testAlphaTexture = true) const;
    
    virtual bool intersect(const Ray &ray, Float *tHit, SurfaceInteraction *isect,
                   bool testAlphaTexture = true) const override;

    virtual bool intersectP(const Ray &ray, bool testAlphaTexture = true) const override;
    
    virtual Float area() const override {
        const Point3f &p0 = _mesh->points[_vertexIdx[0].pos];
        const Point3f &p1 = _mesh->points[_vertexIdx[1].pos];
        const Point3f &p2 = _mesh->points[_vertexIdx[2].pos];
        return 0.5 * cross(p1 - p0, p2 - p0).length();
    }
    
    virtual Interaction samplePos(const Point2f &u, Float *pdf) const override;
    
    /**
     * 先把三角形三个顶点投影到单位球上
     * 球面三角形定理
     * 球面三角形的三个球面角大小分别为ABC，球面三角形的面积为(A+B+C-π)/r^2
     * 所以对应的立体角为(A+B+C-π)
     */
    Float solidAngle(const Point3f &p, int nSamples = 0) const override {
        std::vector<Vector3f> pSphere = {
            normalize(_mesh->points[_vertexIdx[0].pos] - p), normalize(_mesh->points[_vertexIdx[1].pos] - p),
            normalize(_mesh->points[_vertexIdx[2].pos] - p)
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
    
    void getUVs(Point2f uv[3]) const {
        if (_mesh->uv) {
            uv[0] = _vertexIdx[0].uv < 0 ? Point2f(0, 0) : _mesh->uv[_vertexIdx[0].uv];
            uv[1] = _vertexIdx[1].uv < 0 ? Point2f(1, 0) : _mesh->uv[_vertexIdx[1].uv];
            uv[2] = _vertexIdx[2].uv < 0 ? Point2f(1, 1) : _mesh->uv[_vertexIdx[2].uv];
        } else {
            uv[0] = Point2f(0, 0);
            uv[1] = Point2f(1, 0);
            uv[2] = Point2f(1, 1);
        }
    }
    
private:
    
    std::shared_ptr<TriangleMesh> _mesh;
    const Index * _vertexIdx;
    int _faceIndex;
};

shared_ptr<TriangleMesh> createTriMesh(const shared_ptr<const Transform> &objectToWorld, int nTriangles,
    const int *vertexIndices, int nVertices, const Point3f *P,
    const Point2f *uv=nullptr, const Normal3f *N=nullptr, const Vector3f *S=nullptr,
    const std::shared_ptr<Texture<Float>> &alphaMask=nullptr,
    const std::shared_ptr<Texture<Float>> &shadowAlphaMask=nullptr,
                                  const int *faceIndices=nullptr);

shared_ptr<Triangle> createTri(const shared_ptr<const Transform> &o2w, shared_ptr<const Transform> w2o,
                            bool reverseOrientation,
                            const std::shared_ptr<TriangleMesh> &_mesh,
                            int triNumber);

vector<shared_ptr<Shape>> createQuad(const shared_ptr<const Transform> &o2w,
                        bool reverseOrientation,
                        Float width, Float height = 0,
                        const MediumInterface &mediumInterface = nullptr);


vector<shared_ptr<Primitive>> createQuadPrimitive(const nloJson &,
                                                  const shared_ptr<const Material>&,
                                                  vector<shared_ptr<Light>> &lights,
                                                  const MediumInterface &mediumInterface);

vector<shared_ptr<Shape>> createCube(const shared_ptr<const Transform> &o2w,
                                     bool reverseOrientation,
                                     Float x, Float y, Float z,
                                     const MediumInterface &mediumInterface);

vector<shared_ptr<Primitive>> createCubePrimitive(const nloJson &data,
                                                  const shared_ptr<const Material>& mat,
                                                  vector<shared_ptr<Light>> &lights,
                                                  const MediumInterface &mediumInterface);

vector<shared_ptr<Primitive>> createModelPrimitive(const nloJson &data,
                                                   const shared_ptr<const Material> &mat,
                                                   vector<shared_ptr<Light>> &lights,
                                                   const MediumInterface &mediumInterface);

vector<shared_ptr<Shape>> createTriFromFile(const string &fn,
                                            const shared_ptr<const Transform> &o2w,
                                            bool reverseOrientation,
                                            const string &basePath = "");

vector<shared_ptr<Primitive>> createPrimitive(const vector<shared_ptr<Shape>> &triLst,
                                              vector<shared_ptr<Light>> &lights,
                                              const shared_ptr<const Material>&mat,
                                              const MediumInterface &mediumInterface,
                                              const nloJson &emissionData);

vector<shared_ptr<Primitive>> getPrimitiveFromObj(const string &fn,
                                                const shared_ptr<const Transform> &o2w,
                                                vector<shared_ptr<Light>> &lights,
                                                const MediumInterface &mediumInterface,
                                                bool reverseOrientation,
                                                const string &basePath = "");

PALADIN_END

#endif /* trianglemesh_hpp */
