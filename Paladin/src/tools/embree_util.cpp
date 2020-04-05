//
//  embree_util.cpp
//  Paladin
//
//  Created by Zero on 2020/4/5.
//

#include "embree_util.hpp"

PALADIN_BEGIN

namespace EmbreeUtil {

static RTCDevice globalDevice = nullptr;

void initDevice() {
    globalDevice = rtcNewDevice(nullptr);
}

RTCDevice getDevice() {
    return globalDevice;
}


}

PALADIN_END
