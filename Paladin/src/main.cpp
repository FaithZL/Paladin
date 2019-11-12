//
//  main.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include <iostream>
#include "core/header.h"
#include "core/paladin.hpp"

#include "alltest/test_lua.hpp"
#include "alltest/test_rapidxml.hpp"
#include "alltest/test_glog.hpp"
#include "alltest/test_openexr.hpp"
#include "alltest/loadfile.h"
#include "alltest/testrender.h"
#include "math/lowdiscrepancy.hpp"
#include "alltest/jsontest.h"
#include "parser/transformcache.h"


USING_PALADIN

USING_STD


int sum(std::initializer_list<int> ls={})
{
    int  m = 0;
    for(auto &n : ls)
    {
           m += n;
    }
    return m;
}

int main(int argc, const char * argv[]) {
    // insert code here...
    COUT << "Hello, paladin!\n";
    
    Paladin paladin;
    paladin.render("res/conelbox.json");
    int b = 1;
    auto a = {1,1,b};
    cout << sum(a) << endl;
    
//    paladin.render("res/scene.json");
    
//    test_lua_main();
//    test_rapidxml_main();
//    loadfile();
//    testscene();
//    test_glog_main(argv[0]);
//    testscene();
//    testjson();
//    testCache();
#ifdef _MSC_VER
    //让VS运行debug时不至于黑屏一闪而过
    system("pause");
#endif    
    return 0;
}

