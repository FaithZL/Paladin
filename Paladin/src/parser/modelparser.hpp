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

PALADIN_BEGIN

using namespace tinyobj;

class ModelParser {
    
public:
    
    ModelParser() {
        
    }
    
    void load(const string &fn, const string &basePath = nullptr,
              bool triangulate = true);
    
    
private:
    
    attrib_t _attrib;
    
    vector<shape_t> _shapes;
    
    vector<material_t> _materials;
    // 用于输出
    vector<shared_ptr<Primitive>> _primitives;
};

PALADIN_END

#endif /* modelparser_hpp */
