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
    
    bool load(const string &fn, const string &basePath = nullptr,
              bool triangulate = true);
    
    void initPoints();
    
    void initNormals();
    
    void initUVs();
    
    void parseShapes();
    
    void parseMesh(const mesh_t &mesh);
    
    void remedyIndices();
    
    vector<shared_ptr<Shape>> getTriLst(const shared_ptr<const Transform> &o2w, bool reverseOrientation);
    
private:
    // tinyobjloader中的成员
    attrib_t _attrib;
    vector<shape_t> _shapes;
    vector<material_t> _materials;
    
    // triangle mesh构造需要的参数
    
    // 三角形个数
    int _nTriangles;
    // 顶点个数
    int _nVertices;
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
    vector<int> _pointIndices;
    // uv索引列表
    vector<int> _uvIndices;
    // 法线索引列表
    vector<int> _normalIndice;
};

PALADIN_END

#endif /* modelparser_hpp */
