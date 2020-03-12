//
//  meshparser.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/3/1.
//

#ifndef meshparser_hpp
#define meshparser_hpp

#include "shapes/trianglemesh.hpp"

PALADIN_BEGIN


/**
 * 为了支持unity场景导出的文件
 * 文件中有mesh格式的数据，包含顶点列表，索引列表
 * 该类负责直接解析mesh数据，创建primitive列表
 */
class MeshParser {
    
public:
    
    MeshParser() {
        
    }
    
    vector<shared_ptr<Primitive>> getPrimitiveLst(const nloJson &data, vector<shared_ptr<Light>> &lights);
    
    
    vector<shared_ptr<Primitive>> getPrimitiveLst(const nloJson &param,
                                                  vector<shared_ptr<Light>> &lights,
                                                  const shared_ptr<Transform> &);
    
    
};

PALADIN_END

#endif /* meshparser_hpp */
