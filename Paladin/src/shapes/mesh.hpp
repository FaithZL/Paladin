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

PALADIN_BEGIN



class Mesh : public Shape {
    
public:
    Mesh(const shared_ptr<const Transform> &objectToWorld, int nTriangles,
                        const vector<IndexSet> &vertexIndices, const vector<Point3f> *P,
                        const vector<Normal3f> *N=nullptr,
                        const vector<Point2f> *UV=nullptr, const vector<Vector3f> *E=nullptr,
                        const std::shared_ptr<Texture<Float>> &alphaMask=nullptr,
                        const std::shared_ptr<Texture<Float>> &shadowAlphaMask=nullptr,
                        const int *faceIndices=nullptr);
    
    static std::shared_ptr<Mesh> create(const shared_ptr<const Transform> &objectToWorld, int nTriangles,
                                        const vector<IndexSet> &vertexIndices, const vector<Point3f> *P,
                                        const vector<Normal3f> *N=nullptr, const vector<Point2f> *UV=nullptr,
                                        const vector<Vector3f> *E=nullptr,
                                        const std::shared_ptr<Texture<Float>> &alphaMask=nullptr,
                                        const std::shared_ptr<Texture<Float>> &shadowAlphaMask=nullptr,
                                        const int *faceIndices=nullptr);
    
     
    
private:
    
    // 三角形个数，顶点个数
    const int nTriangles, nVertices;
    
    vector<IndexSet> vertexIndice;
    
    vector<TriangleI *> _triangles;
    
    // 法线索引
    std::vector<int> normalIndices;
    // 纹理索引
    std::vector<int> uvIndices;
};

PALADIN_END

#endif /* mesh_hpp */
