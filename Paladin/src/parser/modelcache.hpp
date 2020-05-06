//
//  modelcache.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/3/12.
//

#ifndef modelcache_hpp
#define modelcache_hpp

#include "core/header.h"
#include "shapes/trianglemesh.hpp"
#include "shapes/mesh.hpp"

PALADIN_BEGIN

class ModelCache {
    
public:
    
    static ModelCache * getInstance();
    
    vector<shared_ptr<Primitive>> getPrimitives(const string &fn,
                                                const Transform *transform,
                                                vector<shared_ptr<Light>> &lights);
    
    static vector<shared_ptr<Primitive>> createPrimitive(const nloJson &meshData,
                                                  const Transform *transform,
                                                  vector<shared_ptr<Light>> &lights,
                                                  mutex * mtx = nullptr);
    
    vector<shared_ptr<Primitive>> loadPrimitives(const string &fn,
                                                 const Transform *transform,
                                                 vector<shared_ptr<Light>> &lights);
    
    
    
    static vector<shared_ptr<Shape>> getShapes(const string &fn,
                                        const Transform *transform,
                                        vector<shared_ptr<Light>> &lights);
    
private:
    
    vector<shared_ptr<Shape>> loadShapes(const string &fn,
                                         const Transform *transform,
                                         vector<shared_ptr<Light>> &lights);
    
    vector<shared_ptr<Shape>> createShapes(const nloJson &meshData,
                                           const Transform *transform,
                                           vector<shared_ptr<Light>> &lights);
    
    static void remedyData(const vector<int> &checkTable,
                           vector<Point3f> &points,
                           vector<Normal3f> &normals,
                           vector<Point2f> &uv,
                           vector<IndexSet> &indice);
    
    
    ModelCache() {
        
    }
    
    static ModelCache * s_modelCache;
    
    map<string, vector<shared_ptr<Primitive>>> _modelMap;
    
    map<string, vector<shared_ptr<Shape>>> _meshMap;
};


PALADIN_END

#endif /* modelcache_hpp */
