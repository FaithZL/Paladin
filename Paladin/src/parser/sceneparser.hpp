//
//  sceneparser.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/11.
//

#ifndef sceneparser_hpp
#define sceneparser_hpp

//#include "renderoption.h"
#include "transformcache.h"
#include <fstream>
PALADIN_BEGIN

USING_STD

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
    
    void parse(const neb::CJsonObject &param);
    
    Sampler * parseSampler(const neb::CJsonObject &param);
    
    Camera * parseCamera(const neb::CJsonObject &param);
    
    Integrator * parseIntegrator(const neb::CJsonObject &param);
    
    Filter * parseFilter(const neb::CJsonObject &param);
    
    Aggregate * parseAccelerator(const neb::CJsonObject &param);
    
    Film * parseFilm(const neb::CJsonObject &param);
    
private:
    
    TransformCache _transformCache;
    
    Float _transformStartTime = 0;
    
    Float _transformEndTime = 0;
    
    string _samplerName = "stratified";
    
    string _integratorName = "pathtracer";
    
    string _filterName = "box";
    
    string _acceleratorName = "bvh";
    
    string _filmName = "image.png";
    
    string _cameraName = "perspective";
    
    vector<shared_ptr<Light>> lights;
    
    vector<shared_ptr<Primitive>> primitives;
    
    shared_ptr<Transform *> cameraToWorld;
};



PALADIN_END

#endif /* sceneparser_hpp */
