//
//  model.h
//  Paladin
//
//  Created by SATAN_Z on 2019/8/4.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef model_h
#define model_h

#include "core/header.h"
#include "triangle.hpp"
#include "tiny_obj_loader.h"

PALADIN_BEGIN

static bool TestLoadObj(const char* filename, const char* basepath = NULL,
                        bool triangulate = true) {
    std::cout << "Loading " << filename << std::endl;
    
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    
    std::string warn;
    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename,
                                basepath, triangulate);
    
    if (!warn.empty()) {
        std::cout << "WARN: " << warn << std::endl;
    }
    
    if (!err.empty()) {
        std::cerr << "ERR: " << err << std::endl;
    }
    
    if (!ret) {
        printf("Failed to load/parse .obj.\n");
        return false;
    }
    
    return true;
}

/*
 连接模型加载与三角形网络的适配器
 */
class Model {
    
public:
    
    Model(std::string &path, std::string &fname) {
        
    }
    
private:
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    
};

PALADIN_END

#endif /* model_h */
