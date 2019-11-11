//
//  jsontest.h
//  Paladin
//
//  Created by SATAN_Z on 2019/11/11.
//

#ifndef jsontest_h
#define jsontest_h

#include "ext/json/CJsonObject.hpp"
#include "core/header.h"


void testjson() {
//    FILE * f = open(
    ifstream myfile;
    myfile.open("res/scene.json", ios::in);
    std::string str;
    stringstream buf;
    buf << myfile.rdbuf();
    str = buf.str();
//    std::cout << str;
    
    neb::CJsonObject j(str);
    COUT << j["bsdfs"][0];
}

#endif /* jsontest_h */
