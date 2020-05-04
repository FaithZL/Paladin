//
//  mesh.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/5/3.
//

#include "mesh.hpp"

PALADIN_BEGIN

Mesh::Mesh(const Transform * objectToWorld,
               const vector<IndexSet> &vertexIndices, const vector<Point3f> *P,
               const vector<Normal3f> *N,
               const vector<Point2f> *UV, const vector<Vector3f> *E,
               const std::shared_ptr<Texture<Float>> &alphaMask,
               const std::shared_ptr<Texture<Float>> &shadowAlphaMask,
                const int *faceIndices)
: Shape(objectToWorld, nullptr, false),
_nTriangles(vertexIndices.size() / 3),
_nVertices(P->size()),
_surfaceArea(-1) {
    
    size_t size = P->size();
    _points.reset(new Point3f[size + 1]);
    for (int i = 0; i < size; ++i) {
        Point3f p = (*P)[i];
        _points[i] = objectToWorld->exec(p);
    }
    
    if (N && N->size() > 0) {
        size_t size = N->size();
        _normals.reset(new Normal3f[size + 1]);
        for (int i = 0; i < size; ++i) {
            _normals[i] = normalize(objectToWorld->exec((*N)[i]));
        }
    } else {
        _normals.reset();
    }
    
    if (UV && UV->size() > 0) {
        size_t size = UV->size();
        _uv.reset(new Point2f[size + 1]);
        memcpy(_uv.get(), &UV->at(0), size * sizeof(Point2f));
    } else {
        _uv.reset();
    }
    
    initial();
}

void Mesh::initial() {
    // 计算表面积，计算面积分布
    vector<Float> areaLst;
    areaLst.reserve(_nTriangles);
    for (size_t i = 0; i < _vertexIndice.size(); i += 3) {
        TriangleI tri(&(_vertexIndice[0]));
        _triangles.push_back(tri);
        Float area = tri.getArea(_points.get());
        _surfaceArea += area;
        areaLst.push_back(area);
    }
    _areaDistrib = Distribution1D(&areaLst[0], _nTriangles);
    _invArea = 1.f / _surfaceArea;
}

Interaction Mesh::samplePos(const Point2f &u, Float *pdf) const {
    DCHECK(_surfaceArea > 0);
    size_t idx = _areaDistrib.sampleContinuous(u.x);
    
    TriangleI tri = _triangles[idx];
    Interaction ret;
    *pdf = pdfPos();
    Point2f uv;
    Normal3f n;
    tri.sample(_points.get(), u);
    return ret;
}

PALADIN_END
