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
#include "shapes/trianglemesh.hpp"

USING_PALADIN

USING_STD

struct Idx {
    int a;
    int _;
    Idx(int a) {
        this->a = a;
        this->_ = a;
    }
};
void tt() {
    EmbreeUtil::initDevice();
    float vert[16] = {
        1,1,0,0,
        -1,1,0,0,
        -1,-1,0,0,
        1,-1,0,0
    };
    
    Point3f ps[4] = {
        Point3f(-1, 1, -1),
        Point3f(-1, -1, -1),
        Point3f(1, -1, -1),
        Point3f(1, 1, -1),
    };
    
    unsigned int indice[] = {
        0,1,2,
        0,2,3
    };
    
    auto rtcScene = rtcNewScene(EmbreeUtil::getDevice());
    RTCGeometry geom = rtcNewGeometry(EmbreeUtil::getDevice(), RTC_GEOMETRY_TYPE_TRIANGLE);
    rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, ps,
            0, 12, 4);
    rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, indice,
            0, sizeof(unsigned int) * 3 , 2);
    
//    rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, is,
//                               0, sizeof(Index) * 3 , 2);
    
    
    
    rtcAttachGeometry(rtcScene, geom);

    rtcCommitGeometry(geom);
    
    rtcCommitScene(rtcScene);
    
    auto r = Ray(Point3f(0.5,0,1),Vector3f(0,0,-1));
    
    RTCIntersectContext context;
    rtcInitIntersectContext(&context);
    RTCRayHit rh = EmbreeUtil::toRTCRayHit(r);
    rtcIntersect1(rtcScene, &context, &rh);
    
    RTCRay rr = EmbreeUtil::convert(r);
    
    RTCIntersectContext context2;
    rtcInitIntersectContext(&context2);
    rtcOccluded1(rtcScene, &context2, &rr);
    
    int gid = rh.hit.geomID;
    int pid = rh.hit.primID;
    if (gid != -1) {
        int a= 0;
    }
    
}

inline float ToFloat(int32_t i) {
    float f;
    memcpy(&f, &i, sizeof(int32_t));
    return f;
}

//inline float ToFloat(int32_t i) {
//    float f;
//    f = reinterpret_cast<float&>(i);
//    return f;
//}

//inline float ToFloat(int32_t i) {
//    union {
//        float f;
//        int32_t i;
//    } ret;
//    ret.i = i;
//    return ret.f;
//}

void test() {
    auto start = clock();
    Vector3f w(0,0.5,0.7);
    Vector3f e(0,0.5,0.7);
    
    for (int i = 0; i < 100000; ++i) {
//        volatile auto tmp = ToFloat(rand());
        Point3f p = Point3f(rand(),rand(),rand());
        Normal3f n = Normal3f(rand(),rand(),rand());
//        volatile auto tmp = paladin::offsetRayOrigin(p, e, n, w);
        volatile auto tmp = paladin::offsetRay(p, n, w);
    }
    auto end = clock();
    cout << end - start << endl;
}

int main(int argc, const char * argv[]) {
    google::InitGoogleLogging(argv[0]);
    COUT << "Hello, paladin!\n";
//    test();
//    glfwInit();
    
    
//    tt();
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

