//
//  jsontest.h
//  Paladin
//
//  Created by SATAN_Z on 2019/11/11.
//

#ifndef jsontest_h
#define jsontest_h


#include "core/header.h"

PALADIN_BEGIN

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
    neb::CJsonObject j2;

}

PALADIN_END

#endif /* jsontest_h */
