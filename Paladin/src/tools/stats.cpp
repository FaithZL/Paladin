//
//  stats.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/1/5.
//

#include "stats.hpp"

PALADIN_BEGIN

Stats * Stats::s_stats = nullptr;

Stats * Stats::getInstance() {
    if (s_stats == nullptr) {
        s_stats = new Stats();
    }
    return s_stats;
}

PALADIN_END
