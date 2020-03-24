//
//  paladin.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "paladin.hpp"

PALADIN_BEGIN

Paladin * Paladin::s_paladin = nullptr;

Paladin * Paladin::getInstance() {
    if (s_paladin == nullptr) {
        s_paladin = new Paladin();
    }
    return s_paladin;
}

shared_ptr<const Medium> Paladin::getGlobalMedium() {
    return _sceneParser.getGlobalMedium();
}

PALADIN_END
