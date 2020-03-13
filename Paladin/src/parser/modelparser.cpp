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
#include "textures/imagemap.hpp"
#include "materials/matte.hpp"

PALADIN_BEGIN

bool ModelParser::load(const string &fn, const string &basePath, bool triangulate) {
    string warn;
    string err;
    string mtlDir = basePath.empty()
                ? fn.substr(0, fn.find_last_of("/"))
                :basePath;
    
    _basePath = mtlDir;
    
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

void ModelParser::generateNormals() {
    map<string, Float> areaMap;
    vector<Float> areaLst;
    areaLst.resize(_points.size(), 0);
    _normals.resize(_points.size(), Normal3f(0, 0, 0));
    for (size_t i = 0; i < _verts.size(); i += 3) {
        int posIdx0 = _verts[i].pos;
        int posIdx1 = _verts[i + 1].pos;
        int posIdx2 = _verts[i + 2].pos;
        Point3f p0 = _points[posIdx0];
        Point3f p1 = _points[posIdx1];
        Point3f p2 = _points[posIdx2];
        Vector3f crs = cross(p1 - p0, p2 - p0);
        Float area = 0.5 * crs.length();
        Normal3f normal = normalize(Normal3f(crs));
        for (size_t j = 0; j < 3; ++j) {
            int posIdx = _verts[i + j].pos;
            Float curArea = areaLst[posIdx];
            Normal3f curNormal = _normals[posIdx];
            Normal3f newNormal = (curNormal * curArea + normal * area) / (area + curArea);
            areaLst[posIdx] = area + curArea;
            _normals[posIdx] = newNormal;
        }
    }
    return;
}

void ModelParser::parseShapes() {
    for (size_t i = 0; i < _shapes.size(); ++i) {
        shape_t shape = _shapes.at(i);
        parseMesh(shape.mesh);
    }
}

bool isValid(Float rgb[3]) {
    return rgb[0] > 0 || rgb[1] > 0 || rgb[2] > 0;
}

shared_ptr<Texture<Spectrum>> createKd(const material_t &mat, const string &basePath) {
    if (!mat.diffuse_texname.empty()) {
        string fn = basePath + "/" + mat.diffuse_texname;
        return createImageMap(fn);
    }
    return ConstantTexture<Spectrum>::create(mat.diffuse);
}

shared_ptr<Texture<Spectrum>> createNormalMap(const material_t &mat, const string &basePath) {
    if (!mat.bump_texname.empty()) {
        string fn = basePath + "/" + mat.bump_texname;
        return createImageMap(fn);
    }
    return nullptr;
}

shared_ptr<Texture<Spectrum>> createKs(const material_t &mat, const string &basePath) {
    if (!mat.specular_texname.empty()) {
        string fn = basePath + "/" + mat.specular_texname;
        return createImageMap(fn);
    }
    return ConstantTexture<Spectrum>::create(mat.specular);
}

shared_ptr<Texture<Spectrum>> createKr(const material_t &mat, const string &basePath) {
    if (!mat.reflection_texname.empty()) {
        string fn = basePath + "/" + mat.reflection_texname;
        return createImageMap(fn);
    }
    return nullptr;
}

SurfaceData ModelParser::fromObjMaterial(const material_t &mat) {
    SurfaceData ret;
    ret.emission[0] = mat.emission[0];
    ret.emission[1] = mat.emission[1];
    ret.emission[2] = mat.emission[2];
    
    shared_ptr<Texture<Spectrum>> Kd = createKd(mat, _basePath);
    shared_ptr<Texture<Spectrum>> Ks = createKs(mat, _basePath);
    shared_ptr<Texture<Spectrum>> Kt = ConstantTexture<Spectrum>::create(mat.transmittance);
    shared_ptr<Texture<Spectrum>> Kr = createKr(mat, _basePath);
    shared_ptr<Texture<Spectrum>> op = ConstantTexture<Spectrum>::create(mat.dissolve);
    shared_ptr<Texture<Float>> eta = ConstantTexture<Float>::create(mat.ior);
    float roughness = (mat.shininess == 0) ? 0. : (1.f / mat.shininess);
    shared_ptr<Texture<Float>> rough = ConstantTexture<Float>::create(roughness);
    shared_ptr<Texture<Spectrum>> normalMap = createNormalMap(mat, _basePath);
    
    ret.material = HyperMaterial::create(Kd, Ks, Kr, Kt, rough,
                                         nullptr, nullptr, op,
                                         eta, nullptr, normalMap);
    return ret;
}

void ModelParser::parseMaterials() {
    for (size_t i = 0; i < _materials.size(); ++i) {
        _materialLst.push_back(fromObjMaterial(_materials[i]));
    }
}

void ModelParser::parseMesh(const mesh_t &mesh) {
    for (size_t i = 0; i < mesh.indices.size(); ++i) {
        index_t idx = mesh.indices[i];
        Index index;
        index.pos = idx.vertex_index;
        index.normal = idx.normal_index;
        index.uv = idx.texcoord_index;
        _verts.push_back(index);
    }
}

vector<shared_ptr<Shape>> ModelParser::getTriLst(const shared_ptr<const Transform> &o2w,
                                                 bool reverseOrientation) {
    packageData();
    parseShapes();
    if (_normals.size() == 0) {
        generateNormals();
    }
    size_t nTriangles = _verts.size() / 3;
    
    auto mesh = TriangleMesh::create(o2w, nTriangles, _verts, &_points, &_normals, &_UVs);

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
        for (int j = 0; j < 3; ++j) {
            index_t idx = mesh.indices[i * 3 + j];
            idx.normal_index = idx.normal_index == -1 ? idx.vertex_index : idx.normal_index;
            _verts.emplace_back(idx.texcoord_index, idx.vertex_index, idx.normal_index, 0);
        }
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
    if (_normals.size() == 0) {
        generateNormals();
    }
    size_t nTriangles = _verts.size() / 3;
    auto mesh = TriangleMesh::create(o2w, nTriangles, _verts, &_points, &_normals, &_UVs);
    shared_ptr<Transform> w2o(o2w->getInverse_ptr());
    for (size_t i = 0; i < nTriangles; ++i) {
        auto tri = createTri(o2w, w2o, reverseOrientation, mesh, i);
        int matIdx = _matIndices[i];
        SurfaceData data;
        shared_ptr<DiffuseAreaLight> light;
        shared_ptr<GeometricPrimitive> prim;
        if (matIdx >= 0) {
            data = _materialLst[matIdx];
            light = DiffuseAreaLight::create(data.emission, tri, mediumInterface);
            // 如果primitive为光源，则材质为光源默认材质
            auto mat = light ? createLightMat() : data.material;
            prim = GeometricPrimitive::create(tri, mat, light, mediumInterface);
            if (light) {
                lights.push_back(light);
            }
        } else {
            prim = GeometricPrimitive::create(tri, nullptr, nullptr, mediumInterface);
        }
        
        ret.push_back(prim);
    }
    
    return ret;
}

PALADIN_END
