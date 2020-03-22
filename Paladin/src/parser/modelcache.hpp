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

PALADIN_BEGIN

class ModelCache {
    
public:
    
    static ModelCache * getInstance();
    
    vector<shared_ptr<Primitive>> getPrimitives(const string &fn,
                                                const shared_ptr<Transform> &transform,
                                                vector<shared_ptr<Light>> &lights);
    
    static vector<shared_ptr<Primitive>> createPrimitive(const nloJson &meshData,
                                                  const shared_ptr<const Transform> &transform,
                                                  vector<shared_ptr<Light>> &lights);
    
    vector<shared_ptr<Primitive>> loadPrimitives(const string &fn,
                                                 const shared_ptr< Transform> &transform,
                                                 vector<shared_ptr<Light>> &lights);
    
private:
    
    
    
    ModelCache() {
        
    }
    
    static ModelCache * s_modelCache;
    
    map<string, vector<shared_ptr<Primitive>>> _modelMap;
};


PALADIN_END

#endif /* modelcache_hpp */
