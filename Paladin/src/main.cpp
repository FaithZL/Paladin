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

#include "GLFW/glfw3.h"
#include "tools/embree_util.hpp"

USING_PALADIN

USING_STD

void tt() {
    EmbreeUtil::initDevice();
    Float vert[] = {
        1,1,0,
        -1,1,0,
        -1,-1,0,
        1,-1,0
    };
    int indice[] = {
        0,1,2,
        0,2,3
    };
    auto rtcScene = rtcNewScene(EmbreeUtil::getDevice());
    RTCGeometry geom = rtcNewGeometry(EmbreeUtil::getDevice(), RTC_GEOMETRY_TYPE_TRIANGLE);
    rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, vert,
            0, sizeof(Float) * 3, 4);
    rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, indice,
            0, sizeof(int), 6);
    
    rtcAttachGeometry(rtcScene, geom);

    rtcCommitGeometry(geom);
    
    rtcCommitScene(rtcScene);
    
    auto r = Ray(Point3f(0,0,1),Vector3f(0,0,-1));
    
    auto ray = EmbreeUtil::convert(r);
    RTCIntersectContext context;
    rtcInitIntersectContext(&context);
    RTCRayHit rh;
    rh.ray = ray;
    rh.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    rh.hit.primID = RTC_INVALID_GEOMETRY_ID;
    rtcIntersect1(rtcScene, &context, &rh);
    int gid = rh.hit.geomID;
    int pid = rh.hit.primID;
    if (gid != RTC_INVALID_GEOMETRY_ID) {
        int a= 0;
    }
    
}

int main(int argc, const char * argv[]) {
    // insert code here...
    COUT << "Hello, paladin!\n";
//    testscene();
//    glfwInit();
    
    tt();
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

