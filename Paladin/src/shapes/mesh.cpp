//
//  mesh.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/5/3.
//

#include "mesh.hpp"

PALADIN_BEGIN

Mesh::Mesh(const Transform * objectToWorld,
                const vector<IndexSet> &vertexIndices,
                const vector<Point3f> *P,
                const vector<Normal3f> *N,
                const vector<Point2f> *UV,
                const MediumInterface &mi)
: Shape(objectToWorld, nullptr, false,mi, true),
_nTriangles(vertexIndices.size() / 3),
_nVertices(P->size()),
_surfaceArea(-1) {
    _vertexIndice = vertexIndices;
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
        TriangleI tri(&_vertexIndice[i], this);
        _triangles.push_back(tri);
        Float area = tri.getArea(_points.get());
        _surfaceArea += area;
        areaLst.push_back(area);
    }
    _areaDistrib = Distribution1D(&areaLst[0], _nTriangles);
    _invArea = 1.f / _surfaceArea;
    computeWorldBound();
}



RTCGeometry Mesh::rtcGeometry(Scene *scene) const {
    RTCGeometry geom = rtcNewGeometry(EmbreeUtil::getDevice(), RTC_GEOMETRY_TYPE_TRIANGLE);
    
    rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0,
                               RTC_FORMAT_FLOAT3,
                               getVertice(), 0,
                               getVerticeStride(),
                               getVerticeNum());
    
    rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0,
                                RTC_FORMAT_UINT3,
                                getIndice(), 0,
                                getIndiceStride() * 3,
                                getIndiceNum() / 3);
    rtcCommitGeometry(geom);
    return geom;
}

Interaction Mesh::samplePos(const Point2f &u, Float *pdf) const {
    DCHECK(_surfaceArea > 0);
    Point2f _u(u);
    size_t idx = _areaDistrib.sampleDiscrete(_u.x, nullptr, &_u.x);
    
    TriangleI tri = _triangles[idx];
    Interaction ret;
    *pdf = pdfPos();
    tri.sample(_points.get(), _u);
    return ret;
}

void Mesh::computeWorldBound() {
    for (size_t i = 0; i < _triangles.size(); ++i) {
        TriangleI tri = _triangles[i];
        _worldBound = unionSet(_worldBound, tri.worldBound(_points.get()));
    }
}

shared_ptr<Mesh> Mesh::createQuad(const Transform *o2w, Float width, Float height, const MediumInterface &mi) {
    height = height == 0 ? width : height;
    width /= 2.f;
    height /= 2.f;
    DCHECK(width > 0);
    DCHECK(height > 0);
    Point3f tr(width, height, 0);
    Point3f tl(-width, height, 0);
    Point3f br(width, -height, 0);
    Point3f bl(-width, -height,0);
    auto points = vector<Point3f>{tl, bl, br, tr};
    auto vertIndice = vector<int>{0,1,2, 0,2,3};
    auto UVs = vector<Point2f>{Point2f(0, 1), Point2f(0, 0), Point2f(1, 0), Point2f(1,1)};
    int nTri = 2;
    int nVert = 4;
    
    auto ret = Mesh::create(o2w, vertIndice, &points, nullptr, &UVs);
    return ret;
}

shared_ptr<Mesh> Mesh::createCube(const Transform *o2w, Float x, Float y, Float z, const MediumInterface &mi) {
    y = y == 0 ? x : y;
    z = z == 0 ? y : z;
    x = x / 2.f;
    y = y / 2.f;
    z = z / 2.f;
    auto points = vector<Point3f>{
        Point3f(x, y, z),   // 0
        Point3f(-x, y, z),  // 1
        Point3f(x, -y, z),  // 2
        Point3f(x, y, -z),  // 3
        Point3f(-x, -y, z), // 4
        Point3f(x, -y, -z), // 5
        Point3f(-x, y, -z), // 6
        Point3f(-x, -y, -z) // 7
    };
    auto UVs = vector<Point2f>{
        Point2f(0, 1),  // 0
        Point2f(0, 0),  // 1
        Point2f(1, 0),  // 2
        Point2f(1, 1)    // 3
    };
    auto normals = vector<Normal3f>{
        Normal3f(1,0,0),  // 0
        Normal3f(-1,0,0), // 1
        Normal3f(0,1,0),  // 2
        Normal3f(0,-1,0), // 3
        Normal3f(0,0,1),  // 4
        Normal3f(0,0,-1)  // 5
    };
    
    auto Indice = vector<IndexSet>{
        IndexSet(0, 3, 4), IndexSet(1, 0, 4), IndexSet(2, 2, 4), IndexSet(4, 1, 4), // + z
        IndexSet(3, 3, 5), IndexSet(5, 2, 5), IndexSet(6, 0, 5), IndexSet(7, 1, 5), // - z
        IndexSet(0, 3, 2), IndexSet(1, 0, 2), IndexSet(3, 2, 2), IndexSet(6, 1, 2), // + y
        IndexSet(2, 3, 3), IndexSet(4, 0, 3), IndexSet(5, 2, 3), IndexSet(7, 1, 3), // - y
        IndexSet(0, 3, 0), IndexSet(2, 0, 0), IndexSet(3, 2, 0), IndexSet(5, 1, 0), // + x
        IndexSet(1, 3, 1), IndexSet(4, 0, 1), IndexSet(6, 2, 1), IndexSet(7, 1, 1)  // - x
    };
    
    auto ret = Mesh::create(o2w, Indice, &points, &normals, &UVs, mi);
    return ret;
}

PALADIN_END
