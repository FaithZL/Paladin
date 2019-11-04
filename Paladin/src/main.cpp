//
//  main.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include <iostream>
#include "core/paladin.hpp"

#include "alltest/test_lua.hpp"
#include "alltest/test_rapidxml.hpp"
#include "alltest/test_glog.hpp"
#include "alltest/loadfile.h"
#include "alltest/testrender.h"
#include "math/lowdiscrepancy.hpp"

USING_PALADIN

USING_STD


int main(int argc, const char * argv[]) {
    // insert code here...
    COUT << "Hello, paladin!\n";
    Paladin pld;
    
//    test_lua_main();
//    test_rapidxml_main();
//    loadfile();
//    testscene();
    test_glog_main(argv[0]);
#ifdef _MSC_VER
    //让VS运行debug时不至于黑屏一闪而过
    system("pause");
#endif    
    return pld.run(argc, argv);
}

