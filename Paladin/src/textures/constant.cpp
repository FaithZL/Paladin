//
//  constant.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/21.
//

#include "constant.hpp"

PALADIN_BEGIN


//    "param" : 0
CObject_ptr createFloatConstant(const nloJson &param, const Arguments &lst) {
    Float value = param;
    return new ConstantTexture<Float>(value);
}

//"param" : {
//    "colorType" : 0,
//    "color" : [0.1, 0.9, 0.5],
//}
CObject_ptr createSpectrumConstant(const nloJson &param, const Arguments &lst) {
    int colorType = param.value("colorType", 0);
    nloJson color = param.value("color", nloJson::array({1.f, 1.f, 1.f}));
    Float arr[3] = {};
    for (int i = 0; i < 3; ++i) {
        arr[i] = color[i];
    }
    Spectrum spd = Spectrum::FromRGB(arr, (SpectrumType)colorType);
    return new ConstantTexture<Spectrum>(spd);
}

REGISTER("Floatconstant", createFloatConstant)

REGISTER("Spectrumconstant", createSpectrumConstant)

PALADIN_END
