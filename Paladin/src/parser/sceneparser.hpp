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
    
    void parse(const nebJson &);
    
    shared_ptr<Sampler> parseSampler(const nebJson &param);
    
    shared_ptr<Camera> parseCamera(const nebJson &param);
    
    unique_ptr<Integrator> parseIntegrator(const neb::CJsonObject &param);
    
    shared_ptr<Filter> parseFilter(const neb::CJsonObject &);
    
    shared_ptr<Aggregate> parseAccelerator(const neb::CJsonObject &param);
    
    shared_ptr<Film> parseFilm(const neb::CJsonObject &param);
    
private:
    
    TransformCache _transformCache;
    
    shared_ptr<Camera> _camera;
    
    shared_ptr<Filter> _filter;

    shared_ptr<Sampler> _sampler;
    
    shared_ptr<Film> _film;
    
    shared_ptr<Aggregate> _aggregate;
    
    unique_ptr<Integrator> _integrator;
    
    unique_ptr<Scene> _scene;
    
    vector<shared_ptr<Light>> lights;
    
    vector<shared_ptr<Primitive>> primitives;
    
    Transform _cameraToWorld;
};



PALADIN_END

#endif /* sceneparser_hpp */
