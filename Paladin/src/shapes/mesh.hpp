//
//  mesh.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/5/3.
//

#ifndef mesh_hpp
#define mesh_hpp

#include "core/header.h"
#include "core/shape.hpp"
#include "shapes/triangle.hpp"
#include "math/sampling.hpp"
#include "core/material.hpp"

PALADIN_BEGIN



class Mesh : public Shape {
    
public:
    Mesh(const Transform * objectToWorld,
                        const vector<IndexSet> &vertexIndices,
                        const vector<Point3f> *P,
                        const vector<Normal3f> *N = nullptr,
                        const vector<Point2f> *UV = nullptr,
                        const shared_ptr<const Material> &mat = nullptr,
                        const MediumInterface &mi = nullptr);
    
    static shared_ptr<Mesh> create(const Transform * objectToWorld,
                                        const vector<IndexSet> &vertexIndices,
                                        const vector<Point3f> *P,
                                        const vector<Normal3f> *N=nullptr,
                                        const vector<Point2f> *UV=nullptr,
                                        const shared_ptr<const Material> &mat = nullptr,
                                        const MediumInterface &mi = nullptr) {
        return make_shared<Mesh>(objectToWorld, vertexIndices,
                                 P, N, UV, mat, mi);
    }
    
    
    
    static shared_ptr<Mesh> create(const Transform * objectToWorld,
                                        const vector<int> &vertexIndices,
                                        const vector<Point3f> *P,
                                        const vector<Normal3f> *N=nullptr,
                                        const vector<Point2f> *UV=nullptr,
                                        const shared_ptr<const Material> &mat = nullptr,
                                        const MediumInterface &mi = nullptr) {
        vector<IndexSet> indice;
        indice.reserve(vertexIndices.size());
        for (size_t i = 0; i < vertexIndices.size(); ++i) {
            int idx = vertexIndices[i];
            indice.emplace_back(idx);
        }
        return make_shared<Mesh>(objectToWorld, indice,
                                 P, N,UV,mat,mi);
    }
    

    static shared_ptr<Mesh> createQuad(const nloJson &data,
                                        const shared_ptr<const Material>& mat,
                                        vector<shared_ptr<Light>> &lights,
                                       const MediumInterface &mi = nullptr);
    
    static shared_ptr<Mesh> createQuad(const Transform *o2w, Float width, Float height = 0,
                                        const shared_ptr<const Material> &mat = nullptr,
                                            const MediumInterface &mi = nullptr);
    
    static vector<shared_ptr<Mesh>> createModel(const nloJson &data,
                                   const shared_ptr<const Material> &mat,
                                   vector<shared_ptr<Light> > &lights,
                                   const MediumInterface &mi);
    
    static vector<shared_ptr<Mesh>> createMeshes(const nloJson &data,
                                            vector<shared_ptr<Light>> &lights,
                                            const MediumInterface &mi = nullptr);
    
    static shared_ptr<Mesh> createCube(const nloJson &data,
                                        const shared_ptr<const Material>& mat,
                                        vector<shared_ptr<Light>> &lights,
                                       const MediumInterface &mi = nullptr);
    
    static shared_ptr<Mesh> createCube(const Transform *o2w, Float x, Float y = 0, Float z = 0,
                                       const shared_ptr<const Material> &mat = nullptr,
                                            const MediumInterface &mi = nullptr);

    void initial();
    
    RTCGeometry rtcGeometry(Scene *scene) const override;
    
    virtual Float area() const override {
        return _surfaceArea;
    }
    
    F_INLINE const vector<const TriangleI>& getTriangles() const {
        return _triangles;
    }
    
    F_INLINE const TriangleI * getTriangle(uint32_t primID) const {
        return &_triangles[primID];
    }
    
    void computeWorldBound();

    virtual AABB3f worldBound() const override {
        return _worldBound;
    }
    
    virtual AABB3f objectBound() const override {
        return worldToObject->exec(_worldBound);
    }
    
    virtual Interaction samplePos(const Point2f &u, Float *pdf) const override;
    
    virtual void samplePos(PositionSamplingRecord *rcd, const Point2f &u) const override;
    
    F_INLINE Float * getVertice() const {
        return (Float *)_points.get();
    }
    
    F_INLINE size_t getVerticeNum() const {
        return _nVertices;
    }
    
    F_INLINE size_t getVerticeStride() const {
        return sizeof(Point3f);
    }
    
    F_INLINE uint32_t * getIndice() const {
        uint32_t * ret = new uint32_t[_vertexIndice.size()]();
        auto p = ret;
        for (auto iter = _vertexIndice.cbegin(); iter != _vertexIndice.cend(); ++iter) {
            uint32_t pos = iter->pos;
            * p = pos;
            ++p;
        }
        return (ret);
    }
    
    F_INLINE size_t getIndiceNum() const {
        return _vertexIndice.size();
    }
    
    F_INLINE size_t getIndiceStride() const {
        return sizeof(uint32_t);
    }
    
private:
    
    // 三角形个数，顶点个数
    const int _nTriangles, _nVertices;
    
    vector<IndexSet> _vertexIndice;
    
    vector<const TriangleI> _triangles;
    
    Distribution1D _areaDistrib;
    
    // 顶点列表，为世界坐标
    std::unique_ptr<Point3f[]> _points;
    // 法线列表
    std::unique_ptr<Normal3f[]> _normals;
    
    std::unique_ptr<Point2f[]> _uv;
    
    Float _surfaceArea;
    
    AABB3f _worldBound;
    
    friend class TriangleI;
};

PALADIN_END

#endif /* mesh_hpp */
