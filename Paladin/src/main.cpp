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

class A {
    
public:
    int m = 0;
};

void f2(const A * a) {
//    a->m = 2;
    cout <<a->m;
}

class Test {
public:
    Test() {
        p = new A();
        p->m = 9;
    }
    
    void fun() const {
        f2(p);
    }
    
public:
    A * p;
};

int main(int argc, const char * argv[]) {
    // insert code here...
    COUT << "Hello, paladin!\n";
    
//    Test t;
//
//    cout << t.p->m << endl;
//    t.fun();
//    cout << t.p->m << endl;
    
//    Paladin paladin;
//    paladin.render("res/conelbox.json");
//    unique_ptr<int> a;
//    a.reset()
//    paladin.render("res/scene.json");
    
//    test_lua_main();
//    test_rapidxml_main();
//    loadfile();
//    testscene();
//    test_glog_main(argv[0]);
//    testscene();
    testjson();
//    testCache();
#ifdef _MSC_VER
    //让VS运行debug时不至于黑屏一闪而过
    system("pause");
#endif    
    return 0;
}

