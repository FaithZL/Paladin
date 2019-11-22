//
//  bilerp.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/21.
//

#include "bilerp.hpp"

PALADIN_BEGIN

//"param" : {
//    "v00" : 1.0,
//    "v01" : 1.0,
//    "v10" : 1.0,
//    "v11" : 1.0
//}
CObject_ptr createFloatBilerpTexture(const nloJson &param, const Arguments &lst) {
    Float v00 = param.value("v00", 1.f);
    Float v01 = param.value("v01", 1.f);
    Float v10 = param.value("v10", 1.f);
    Float v11 = param.value("v11", 1.f);
    // todo 这里先用默认格式
    auto mapping = unique_ptr<TextureMapping2D>(new UVMapping2D());
    auto ret = new BilerpTexture<Float>(move(mapping), v00, v01, v10, v11);
    return ret;
}

REGISTER("Floatbilerp", createFloatBilerpTexture)

//"param" : {
//    "v00" : [0.1, 0.9, 0.5],
//    "v01" : [0.5, 0.2, 0.5],
//    "v10" : [1.0, 0.9, 0.5],
//    "v11" : [0.1, 0.9, 0.5]
//}
CObject_ptr createSpectrumBilerpTexture(const nloJson &param, const Arguments &lst) {
    nloJson _v00 = param.value("v00", nloJson::array());
    nloJson _v01 = param.value("v01", nloJson::array());
    nloJson _v10 = param.value("v10", nloJson::array());
    nloJson _v11 = param.value("v11", nloJson::array());
    Spectrum v00 = Spectrum::FromJsonRGB(_v00);
    Spectrum v01 = Spectrum::FromJsonRGB(_v01);
    Spectrum v10 = Spectrum::FromJsonRGB(_v10);
    Spectrum v11 = Spectrum::FromJsonRGB(_v11);
    auto mapping = unique_ptr<TextureMapping2D>(new UVMapping2D());
    auto ret = new BilerpTexture<Spectrum>(move(mapping), v00, v01, v10, v11);
    return ret;
}

REGISTER("Spectrumbilerp", createSpectrumBilerpTexture)

PALADIN_END
