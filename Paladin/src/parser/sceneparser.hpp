//
//  sceneparser.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/11.
//

#ifndef sceneparser_hpp
#define sceneparser_hpp

#include "transformcache.h"
#include <fstream>
#include "core/integrator.hpp"


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
        nebJson json(str);
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
    
    void parse(const nebJson &);
    
    void parseShapes(const nebJson &);
    
    Sampler * parseSampler(const nebJson &param);
    
    Camera * parseCamera(const nebJson &, Film *);
    
    Integrator * parseIntegrator(const nebJson &,Sampler * sampler, Camera * camera);
    
    Filter * parseFilter(const nebJson &);
    
    // 解析简单物体，球体，圆柱，圆锥等
    void parseSimpleShape(const nebJson &data, const string &type);
    
    // 解析模型
    void parseModel(const nebJson &data);
    
    shared_ptr<Aggregate> parseAccelerator(const nebJson &);
    
    Film * parseFilm(const nebJson &param, Filter *);
    
private:
    
    TransformCache _transformCache;
    
    shared_ptr<Aggregate> _aggregate;
    
    unique_ptr<Integrator> _integrator;
    
    unique_ptr<Scene> _scene;
    
    vector<shared_ptr<Light>> _lights;
    
    vector<shared_ptr<Primitive>> _primitives;
    
    Transform _cameraToWorld;
};



PALADIN_END

#endif /* sceneparser_hpp */
