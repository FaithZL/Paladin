//
//  mesh.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/5/3.
//

#include "mesh.hpp"
#include "core/material.hpp"
#include "lights/diffuse.hpp"
#include "parser/modelcache.hpp"
#include "tools/stats.hpp"

PALADIN_BEGIN

Mesh::Mesh(const Transform * objectToWorld,
                const vector<IndexSet> &vertexIndices,
                const vector<Point3f> *P,
                const vector<Normal3f> *N,
                const vector<Point2f> *UV,
                const shared_ptr<const Material> &mat,
                const MediumInterface &mi)
: Shape(objectToWorld, nullptr, false,mi, ShapeType::EMesh, mat),
_nTriangles(vertexIndices.size() / 3),
_nVertices(P->size()),
_surfaceArea(0) {
    
    _vertexIndice = vertexIndices;
    size_t size = P->size();
    _points.reset(new Point3f[size + 1]);
    for (int i = 0; i < size; ++i) {
        Point3f p = (*P)[i];
        p = objectToWorld->exec(p);
        _points[i] = p;
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

Mesh::Mesh(const Transform * objectToWorld,
                const nloJson &indice,
                const nloJson &points,
                const nloJson &normals,
                const nloJson &UV,
                const shared_ptr<const Material> &mat,
                const MediumInterface &mi)
:Shape(objectToWorld, nullptr, false,mi, ShapeType::EMesh, mat),
_nTriangles(indice.size() / 3),
_nVertices(points.size() / 3),
_surfaceArea(0) {
    
    size_t size = points.size() / 3;
    _points.reset(new Point3f[size + 1]);
    int i = 0;
    for (auto iter = points.cbegin(); iter != points.cend(); iter += 3) {
        Float x = iter[0];
        Float y = iter[1];
        Float z = iter[2];
        Point3f p(x, y, z);
        _points[i++] = objectToWorld->exec(p);
    }
    
    size = normals.size() / 3;
    if (size > 0) {
        _normals.reset(new Normal3f[size + 1]);
        i = 0;
        for (auto iter = normals.cbegin(); iter != normals.cend(); iter += 3) {
            Float x = iter[0];
            Float y = iter[1];
            Float z = iter[2];
            Normal3f normal(x, y, z);
            _normals[i++] = objectToWorld->exec(normal);
        }
    } else {
        _normals.reset();
    }
    
    size = UV.size() / 2;
    if (size > 0) {
        i = 0;
        _uv.reset(new Point2f[size + 1]);
        for (auto iter = UV.cbegin(); iter != UV.cend(); iter += 2) {
            Float u = iter[0];
            Float v = iter[1];
            _uv[i++] = Point2f(u, v);
        }
    } else {
        _uv.reset();
    }
    
    
    vector<Float> areaLst;
    areaLst.reserve(_nTriangles);
    _vertexIndice.reserve(indice.size());
    for (int i = 0; i < indice.size(); i += 3) {
        _vertexIndice.emplace_back(indice[i]);
        _vertexIndice.emplace_back(indice[i + 1]);
        _vertexIndice.emplace_back(indice[i + 2]);
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

vector<shared_ptr<Mesh>> Mesh::createMeshes(const nloJson &data,
                                            vector<shared_ptr<Light> > &lights,
                                            const MediumInterface &mi) {
    nloJson transformData = data.value("transform", nloJson());
    Transform * transform = createTransform(transformData);
    
    shared_ptr<const Material> mat(nullptr);
    
    nloJson param = data.value("param", nloJson());
    return param.is_string() ?
        ModelCache::getMeshes(param, transform, lights):
        ModelCache::createMeshes(param, transform, lights, mat, mi);
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
    ret = tri.sample(_u);
    return ret;
}

void Mesh::samplePos(PositionSamplingRecord *rcd, const Point2f &u) const {
    DCHECK(_surfaceArea > 0);
    Point2f _u(u);
    size_t idx = _areaDistrib.sampleDiscrete(_u.x, nullptr, &_u.x);
    
    TriangleI tri = _triangles[idx];
    Point2f uv;
    Normal3f normal;
    Point3f pos;
    
    pos = tri.sample(_points.get(), _normals.get(), _uv.get(),
                     &normal, &uv, _u);
    rcd->setGeometry(pos, normal, uv, pdfPos());
}

void Mesh::computeWorldBound() {
    for (size_t i = 0; i < _triangles.size(); ++i) {
        TriangleI tri = _triangles[i];
        _worldBound = unionSet(_worldBound, tri.worldBound(_points.get()));
    }
}

//"param" : {
//    "transform" : {
//        "type" : "translate",
//        "param" : [0,0,0]
//    },
//    "width" : 1,
//    "height" : 1,
//    "reverseOrientation" : false
//}
shared_ptr<Mesh> Mesh::createQuad(const nloJson &data,
                                  const shared_ptr<const Material> &mat,
                                  vector<shared_ptr<Light> > &lights,
                                  const MediumInterface &mi) {
    nloJson param = data["param"];
    auto l2w = createTransform(param.value("transform", nloJson()));
    bool ro = param.value("reverseOrientation", false);
    Float width = param.value("width", 1.f);
    Float height = param.value("height", width);
    auto ret = createQuad(l2w, width, height, mat, mi);
    nloJson emissionData = data.value("emission", nloJson());
    shared_ptr<DiffuseAreaLight> areaLight(createDiffuseAreaLight(emissionData, ret, mi));
    if (areaLight) {
        lights.push_back(areaLight);
        ret->setAreaLight(areaLight);
    }
    return ret;
}

shared_ptr<Mesh> Mesh::createQuad(const Transform *o2w, Float width, Float height,
                                  const shared_ptr<const Material> &mat, const MediumInterface &mi) {
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
    auto ret = Mesh::create(o2w, vertIndice, &points, nullptr, &UVs, mat, mi);
    return ret;
}

static vector<shared_ptr<Mesh>> createModel(const nloJson &data,
                            const shared_ptr<const Material> &mat,
                            vector<shared_ptr<Light> > &lights,
                               const MediumInterface &mi) {
    // todo 创建模型
}

//data : {
//    "type" : "mesh",
//    "subType" : "cube",
//    "name" : "cube1",
//    "enable" : true,
//    "param" : {
//        "transform" :[
//            {
//                "type" : "rotateY",
//                "param" : [-20]
//            },
//            {
//                "type" : "translate",
//                "param" : [-0.3,-0.4,0.2]
//            }
//        ],
//        "x" : 0.6,
//        "y" : 1.2,
//        "z" : 0.6
//    },
//    "material" : "matte1"
//}
shared_ptr<Mesh> Mesh::createCube(const nloJson &data,
                                  const shared_ptr<const Material> &mat,
                                  vector<shared_ptr<Light> > &lights,
                                  const MediumInterface &mi) {
    nloJson param = data.value("param", nloJson::object());
    auto l2w = createTransform(param.value("transform", nloJson()));
    bool ro = param.value("reverseOrientation", false);
    auto o2w(l2w);
    Float x = param.value("x", 1.f);
    Float y = param.value("y", x);
    Float z = param.value("z", y);
    auto ret = createCube(l2w, x, y, z, mat, mi);
    nloJson emissionData = data.value("emission", nloJson());
    shared_ptr<DiffuseAreaLight> areaLight(createDiffuseAreaLight(emissionData, ret, mi));
    if (areaLight) {
        lights.push_back(areaLight);
        ret->setAreaLight(areaLight);
    }
    return ret;
}

shared_ptr<Mesh> Mesh::createCube(const Transform *o2w, Float x, Float y, Float z,
                                  const shared_ptr<const Material> &mat, const MediumInterface &mi) {
    y = y == 0 ? x : y;
    z = z == 0 ? y : z;
    x = x / 2.f;
    y = y / 2.f;
    z = z / 2.f;
    auto points = vector<Point3f>{
        Point3f(-x, -y, z),   // 0
        Point3f(x, -y, z),  // 1
        Point3f(-x, y, z),  // 2
        Point3f(x, y, z),  // 3
        Point3f(-x, y, -z), // 4
        Point3f(x, y, -z), // 5
        Point3f(-x, -y, -z), // 6
        Point3f(x, -y, -z) // 7
    };
    auto UVs = vector<Point2f>{
        Point2f(0, 0),  // 0
        Point2f(1, 0),  // 1
        Point2f(0, 1),  // 2
        Point2f(1, 1)    // 3
    };
    auto normals = vector<Normal3f>{
        Normal3f(0,0,1),  // 0
        Normal3f(0,1,0), // 1
        Normal3f(0,0,-1),  // 2
        Normal3f(0,-1,0), // 3
        Normal3f(1,0,0),  // 4
        Normal3f(-1,0,0)  // 5
    };
    

    auto Indice = vector<IndexSet>{
        IndexSet(0, 0, 0), IndexSet(1, 1, 0), IndexSet(2, 2, 0), IndexSet(2, 2, 0), IndexSet(1, 1, 0), IndexSet(3, 3, 0), // + z
        IndexSet(2, 0, 1), IndexSet(3, 1, 1), IndexSet(4, 2, 1), IndexSet(4, 2, 1), IndexSet(3, 1, 1), IndexSet(5, 3, 1), // + y
        IndexSet(4, 3, 2), IndexSet(5, 2, 2), IndexSet(6, 1, 2), IndexSet(6, 1, 2), IndexSet(5, 2, 2), IndexSet(7, 0, 2), // - z
        IndexSet(6, 0, 3), IndexSet(7, 1, 3), IndexSet(0, 2, 3), IndexSet(0, 2, 3), IndexSet(7, 1, 3), IndexSet(1, 3, 3), // - y
        IndexSet(1, 0, 4), IndexSet(7, 1, 4), IndexSet(3, 2, 4), IndexSet(3, 2, 4), IndexSet(7, 1, 4), IndexSet(5, 3, 4), // + x
        IndexSet(6, 0, 5), IndexSet(0, 1, 5), IndexSet(4, 2, 5), IndexSet(4, 2, 5), IndexSet(0, 1, 5), IndexSet(2, 3, 5)  // - x
    };
    
    auto ret = Mesh::create(o2w, Indice, &points, &normals, &UVs, mat, mi);
    return ret;
}

PALADIN_END
