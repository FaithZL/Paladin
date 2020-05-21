//
//  modelcache.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/3/12.
//

#ifndef modelcache_hpp
#define modelcache_hpp

#include "core/header.h"
#include "shapes/mesh.hpp"

PALADIN_BEGIN

class ModelCache {
    
public:
    
    static ModelCache * getInstance();
    
//    vector<shared_ptr<Primitive>> getPrimitives(const string &fn,
//                                                const Transform *transform,
//                                                vector<shared_ptr<Light>> &lights);
//
//    static vector<shared_ptr<Primitive>> createPrimitive(const nloJson &meshData,
//                                                  const Transform *transform,
//                                                  vector<shared_ptr<Light>> &lights,
//                                                  mutex * mtx = nullptr);
    
//    vector<shared_ptr<Primitive>> loadPrimitives(const string &fn,
//                                                 const Transform *transform,
//                                                 vector<shared_ptr<Light>> &lights);
//    
//    
    
    static vector<shared_ptr<Mesh>> getMeshes(const string &fn,
                                        const Transform *transform,
                                        vector<shared_ptr<Light>> &lights);
    
    static vector<shared_ptr<Mesh>> createMeshes(const nloJson &param,
                                           const Transform *transform,
                                           vector<shared_ptr<Light>> &lights,
                                           const shared_ptr<const Material> &mat = nullptr,
                                           const MediumInterface &mi = nullptr);
    
private:
    
    vector<shared_ptr<Mesh>> loadMeshes(const string &fn,
                                         const Transform *transform,
                                         vector<shared_ptr<Light>> &lights,
                                         const shared_ptr<const Material> &mat = nullptr,
                                         const MediumInterface &mi = nullptr);
    
    ModelCache() {
        
    }
    
    static ModelCache * s_modelCache;
    
    map<string, vector<shared_ptr<Primitive>>> _modelMap;
    
    map<string, vector<shared_ptr<Mesh>>> _meshMap;
};


PALADIN_END

#endif /* modelcache_hpp */
