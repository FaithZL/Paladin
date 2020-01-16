//
//  modelparser.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/1/1.
//

#include "modelparser.hpp"
#include "core/primitive.hpp"
#include "lights/diffuse.hpp"
#include "materials/hyper.hpp"
#include "textures/constant.hpp"

PALADIN_BEGIN

bool ModelParser::load(const string &fn, const string &basePath, bool triangulate) {
    string warn;
    string err;
    string mtlDir = basePath.empty()
                ? fn.substr(0, fn.find_last_of("/"))
                :basePath;
    bool ret = tinyobj::LoadObj(&_attrib, &_shapes, &_materials,
                     &warn, &err, fn.c_str(),
                     mtlDir.c_str(), triangulate);
    
    if (!warn.empty()) {
        std::cout << "WARN: " << warn << std::endl;
    }
    
    if (!err.empty()) {
        std::cout << "WARN: " << err << std::endl;
    }
    
    return ret;
}

void ModelParser::packageData() {
    // 初始化顶点
    for (size_t i = 0; i < _attrib.vertices.size() / 3; ++i) {
        Float x = _attrib.vertices[i * 3 + 0];
        Float y = _attrib.vertices[i * 3 + 1];
        Float z = _attrib.vertices[i * 3 + 2];
        _points.emplace_back(x,y,z);
    }
    // 初始化法线
    for (size_t i = 0; i < _attrib.normals.size() / 3; ++i) {
        Float x = _attrib.normals[i * 3 + 0];
        Float y = _attrib.normals[i * 3 + 1];
        Float z = _attrib.normals[i * 3 + 2];
        _normals.emplace_back(x, y, z);
    }
    // 初始化纹理坐标
    for (size_t i = 0; i < _attrib.texcoords.size() / 2; ++i) {
        Float u = _attrib.texcoords[i * 2 + 0];
        Float v = _attrib.texcoords[i * 2 + 1];
        _UVs.emplace_back(u, v);
    }
}

void ModelParser::parseShapes() {
    for (size_t i = 0; i < _shapes.size(); ++i) {
        shape_t shape = _shapes.at(i);
        parseMesh(shape.mesh);
    }
}

ModelParser::SurfaceData ModelParser::fromObjMaterial(const material_t &mat) {
    SurfaceData ret;
    ret.emission[0] = mat.emission[0];
    ret.emission[1] = mat.emission[1];
    ret.emission[2] = mat.emission[2];
    
    shared_ptr<Texture<Spectrum>> Kd = ConstantTexture<Spectrum>::create(mat.diffuse);
    shared_ptr<Texture<Spectrum>> Ks = ConstantTexture<Spectrum>::create(mat.specular);
    shared_ptr<Texture<Spectrum>> Kt = ConstantTexture<Spectrum>::create(mat.transmittance);
    shared_ptr<Texture<Spectrum>> Kr = nullptr;
    shared_ptr<Texture<Spectrum>> op = ConstantTexture<Spectrum>::create(mat.dissolve);
    shared_ptr<Texture<Float>> eta = ConstantTexture<Float>::create(mat.ior);
    float roughness = (mat.shininess == 0) ? 0. : (1.f / mat.shininess);
    shared_ptr<Texture<Float>> rough = ConstantTexture<Float>::create(roughness);
    
    ret.material = HyperMaterial::create(Kd, Ks, Kr, Kt, rough, nullptr, nullptr, op, eta, nullptr);
    return ret;
}

void ModelParser::parseMaterials() {
    for (size_t i = 0; i < _materials.size(); ++i) {
        _materialLst.push_back(fromObjMaterial(_materials[i]));
    }
}

void ModelParser::parseMesh(const mesh_t &mesh) {
    // todo 顶点索引与法线索引可能不一致，看看怎么处理
    for (size_t i = 0; i < mesh.indices.size(); ++i) {
        index_t idx = mesh.indices[i];
        _vertIndices.push_back(idx.vertex_index);
    }
}

vector<shared_ptr<Shape>> ModelParser::getTriLst(const shared_ptr<const Transform> &o2w,
                                                 bool reverseOrientation) {
    packageData();
    parseShapes();
    size_t nTriangles = _vertIndices.size() / 3;
    
    auto mesh = createTriMesh(o2w, nTriangles, &_vertIndices[0], _points.size(),
                                &_points[0], &_UVs[0], &_normals[0]);

    vector<shared_ptr<Shape>> ret;
    shared_ptr<Transform> w2o(o2w->getInverse_ptr());
    for (int i = 0; i < nTriangles; ++i) {
        ret.push_back(createTri(o2w, w2o, reverseOrientation, mesh, i));
    }
    return ret;
}

void ModelParser::parseShape(const shape_t &shape) {
    mesh_t mesh = shape.mesh;
    for (size_t i = 0; i < mesh.material_ids.size(); ++i) {
        int matId = mesh.material_ids[i];
        index_t idx0 = mesh.indices[i * 3];
        index_t idx1 = mesh.indices[i * 3 + 1];
        index_t idx2 = mesh.indices[i * 3 + 2];
        _vertIndices.push_back(idx0.vertex_index);
        _vertIndices.push_back(idx1.vertex_index);
        _vertIndices.push_back(idx2.vertex_index);
        _matIndices.push_back(matId);
    }
}

vector<shared_ptr<Primitive>> ModelParser::getPrimitiveLst(const shared_ptr<const Transform> &o2w,
                                                           vector<shared_ptr<Light>> &lights,
                                                           bool reverseOrientation,
                                                           const MediumInterface &mediumInterface) {
    vector<shared_ptr<Primitive>> ret;
    packageData();
    parseMaterials();
    
    for (size_t i = 0; i < _shapes.size(); ++i) {
        shape_t shape = _shapes.at(i);
        parseShape(shape);
    }
    size_t nTriangles = _vertIndices.size() / 3;
    auto mesh = createTriMesh(o2w, nTriangles, &_vertIndices[0], _points.size(),
                              &_points[0], &_UVs[0], &_normals[0]);
    shared_ptr<Transform> w2o(o2w->getInverse_ptr());
    vector<shared_ptr<Shape>> triLst;
    for (size_t i = 0; i < nTriangles; ++i) {
        auto tri = createTri(o2w, w2o, reverseOrientation, mesh, i);
        int matIdx = _matIndices[i];
        SurfaceData data;
        shared_ptr<DiffuseAreaLight> light;
        shared_ptr<GeometricPrimitive> prim;
        if (matIdx >= 0) {
            data = _materialLst[matIdx];
            light = DiffuseAreaLight::create(data.emission, tri, mediumInterface);
            prim = GeometricPrimitive::create(tri, data.material, light, mediumInterface);
        } else {
            prim = GeometricPrimitive::create(tri, nullptr, nullptr, mediumInterface);
        }
        
        ret.push_back(prim);
    }
    
    return ret;
}

PALADIN_END
