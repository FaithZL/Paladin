//
//  bilerp.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/21.
//

#include "bilerp.hpp"

PALADIN_BEGIN

CObject_ptr createFloatBilerpTexture(const nloJson &param, const Arguments &lst) {
    
}

REGISTER("Floatbilerp", createFloatBilerpTexture)

CObject_ptr createSpectrumBilerpTexture(const nloJson &param, const Arguments &lst) {
    
}

REGISTER("Spectrum", createSpectrumBilerpTexture)

PALADIN_END
