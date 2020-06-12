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

#include "ext/tinyobj/tiny_obj_loader.h"

#include "alltest/test_lua.hpp"
#include "alltest/test_glog.hpp"
#include "alltest/test_openexr.hpp"
#include "alltest/loadfile.h"
#include "alltest/testrender.h"
#include "math/lowdiscrepancy.hpp"
#include "alltest/jsontest.h"
#include "parser/transformcache.h"
#include "math/ray.h"
#include "GLFW/glfw3.h"
#include "tools/embree_util.hpp"
#include <glog/logging.h>

USING_PALADIN

USING_STD

int main(int argc, const char * argv[]) {
    google::InitGoogleLogging(argv[0]);
    COUT << "Hello, paladin!\n";
    Paladin * paladin = Paladin::getInstance();
    if (argc >= 2) {
        string fileName(argv[1]);
        paladin->render(fileName);
    }
#ifdef _MSC_VER
    //让VS运行debug时不至于黑屏一闪而过
    system("pause");
#endif    
    return 0;
}

