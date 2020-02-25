//
//  unityparser.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/2/25.
//

#ifndef unityparser_hpp
#define unityparser_hpp

#include "core/header.h"

PALADIN_BEGIN

/**
 *
 * unity场景解析
 * 支持unity场景解析
 * 
 * 
 */
class UnityParser {
    
public:
    UnityParser() {
        
    }
    
    bool load(const std::string &fn, const std::string &basePath = "",
                    bool triangulate = true);
    
private:
    
};

PALADIN_END

#endif /* unityparser_hpp */
