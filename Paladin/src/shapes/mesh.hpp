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

PALADIN_BEGIN



class Mesh : public Shape {
    
public:
    Mesh(const Transform * objectToWorld,
                        const vector<IndexSet> &vertexIndices, const vector<Point3f> *P,
                        const vector<Normal3f> *N=nullptr,
                        const vector<Point2f> *UV=nullptr, const vector<Vector3f> *E=nullptr,
                        const std::shared_ptr<Texture<Float>> &alphaMask=nullptr,
                        const std::shared_ptr<Texture<Float>> &shadowAlphaMask=nullptr,
                        const int *faceIndices=nullptr);
    
    static std::shared_ptr<Mesh> create(const Transform * objectToWorld,
                                        const vector<IndexSet> &vertexIndices, const vector<Point3f> *P,
                                        const vector<Normal3f> *N=nullptr, const vector<Point2f> *UV=nullptr,
                                        const vector<Vector3f> *E=nullptr,
                                        const std::shared_ptr<Texture<Float>> &alphaMask=nullptr,
                                        const std::shared_ptr<Texture<Float>> &shadowAlphaMask=nullptr,
                                        const int *faceIndices=nullptr) {
        return make_shared<Mesh>(objectToWorld, vertexIndices,
                                 P, N,UV, E,alphaMask,
                                 shadowAlphaMask, faceIndices);
    }
    
    void initial();
    
    virtual Interaction samplePos(const Point2f &u, Float *pdf) const override;
    
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
    
    vector<TriangleI> _triangles;
    
    Distribution1D _areaDistrib;
    
    // 顶点列表，为世界坐标
    std::unique_ptr<Point3f[]> _points;
    // 法线列表
    std::unique_ptr<Normal3f[]> _normals;
    
    std::unique_ptr<Point2f[]> _uv;
    
    Float _surfaceArea;
    
    friend class TriangleI;
};

PALADIN_END

#endif /* mesh_hpp */
