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

USING_STD;

void testjson() {
//    FILE * f = open(
    ifstream myfile;
    myfile.open("res/conelbox.json", ios::in);
    std::string str;
    stringstream buf;
    buf << myfile.rdbuf();
    str = buf.str();
    nloJson j = nloJson::parse(str);
    cout << setw(4) << j;
//    std::cout << str;
//    cout << j.value("transforms", 0);
}

PALADIN_END

#endif /* jsontest_h */
