//
//  sceneparser.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/11.
//

#ifndef sceneparser_hpp
#define sceneparser_hpp

#include "renderoption.h"
#include "transformcache.h"
#include <fstream>
PALADIN_BEGIN



class SceneParser {
    
public:
    void loadFromJson(const std::string &fn) {
        USING_STD;
        std::ifstream fst;
        fst.open(fn.c_str());
        stringstream buffer;
        buffer << fst.rdbuf();
        string str = buffer.str();
        neb::CJsonObject json(str);
        auto errstr = json.GetErrMsg();
        if (errstr != "") {
            COUT << errstr;
            DCHECK(false);
        }
        parse(str);
    }
    
    const TransformCache &getTransformCache() const {
        return _transformCache;
    }
    
    const RenderOption &getRenderOption() const {
        return _renderOption;
    }
    
    void parse(const neb::CJsonObject &jsonObj);
    
    void parseSampler(const neb::CJsonObject &jsonObj);
    
    void parseCamera(const neb::CJsonObject &jsonObj);
    
    void parseIntegrator(const neb::CJsonObject &jsonObj);
    
    void parseFilter(const neb::CJsonObject &jsonObj);
    
    void parseAccelerator(const neb::CJsonObject &jsonObj);
    
    void parseFilm(const neb::CJsonObject &jsonObj);
    
private:
    
    TransformCache _transformCache;
    
    RenderOption _renderOption;
};



PALADIN_END

#endif /* sceneparser_hpp */
