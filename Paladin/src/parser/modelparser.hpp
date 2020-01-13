//
//  modelparser.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/1/1.
//

#ifndef modelparser_hpp
#define modelparser_hpp

#include "core/header.h"
#include "ext/tinyobj/tiny_obj_loader.h"
#include "shapes/trianglemesh.hpp"
#include "math/transform.hpp"
#include "shapes/trianglemesh.hpp"

PALADIN_BEGIN

using namespace tinyobj;



class ModelParser {
    
public:
    
    ModelParser() {
        
    }
    
    struct SurfaceData {
        shared_ptr<const Material> material;
        Float emission[3];
    };
    
    bool load(const string &fn, const string &basePath = "",
              bool triangulate = true);
    
    void parseShapes();
    
    void parseMaterials();
    
    SurfaceData fromObjMaterial(const material_t &);
    
    void parseMesh(const mesh_t &mesh);
    
    void packageData();
    
    void parseShape(const shape_t &);
    
    vector<shared_ptr<Shape>> getTriLst(const shared_ptr<const Transform> &o2w,
                                        bool reverseOrientation);
    
    vector<shared_ptr<Primitive>> getPrimitiveLst(const shared_ptr<const Transform> &o2w,
                                                  vector<shared_ptr<Light>> &lights,
                                                  bool reverseOrientation,
                                                  const MediumInterface &mediumInterface);
    
private:
    // tinyobjloader中的成员
    attrib_t _attrib;
    vector<shape_t> _shapes;
    vector<material_t> _materials;
    
    // triangle mesh构造需要的参数
    
    // 顶点列表
    vector<Point3f> _points;
    // 边向量列表
    vector<Vector3f> _edges;
    // 法线列表
    vector<Normal3f> _normals;
    // uv坐标列表
    vector<Point2f> _UVs;
    // 面索引列表
    vector<int> _faceIndices;
    // 点索引列表
    vector<int> _vertIndices;
    // 材质列表
    vector<SurfaceData> _materialLst;
    // 材质索引列表
    vector<int> _matIndices;
};

PALADIN_END

#endif /* modelparser_hpp */
