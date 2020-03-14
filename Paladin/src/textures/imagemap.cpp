//
//  imagemap.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/1.
//

#include "imagemap.hpp"

PALADIN_BEGIN

template <typename Tmemory, typename Treturn>
std::map<TexInfo, std::unique_ptr<MIPMap<Tmemory>>>
    ImageTexture<Tmemory, Treturn>::_imageCache;

shared_ptr<ImageTexture<RGBSpectrum, Spectrum>> createImageMap(const string &filename, bool gamma, bool doTri,
                                                                    Float maxAniso, ImageWrap wm, Float scale,
                                                                    bool doFilter,
                                                                    unique_ptr<TextureMapping2D> mapping) {
    return make_shared<ImageTexture<RGBSpectrum, Spectrum>>(move(mapping),
                                                            filename,
                                                            doTri, maxAniso,
                                                            wm, scale, gamma,doFilter);
}


template class ImageTexture<RGBSpectrum, Spectrum>;
//"param" : {
//    "fileName" : "res/planet_Quom1200.png",
//    "doTri" : true,
//    "maxAniso" : 8,
//    "wrapMode" : 0,
//    "scale" : 1,
//    "gamma" : false
//}
CObject_ptr createImageMap(const nloJson &param, const Arguments &lst) {
    auto mapping = unique_ptr<TextureMapping2D>(new UVMapping2D());
    string fileName = param.value("fileName", "");
    bool doTri = param.value("doTri", true);
    Float maxAniso = param.value("maxAniso", 8.f);
    int wrapMode = param.value("wrapMode", 0);
    Float scale = param.value("scale", 1.f);
    bool gamma = param.value("gamma", false);
    bool doFilter = param.value("doFilter", true);
    auto ret = new ImageTexture<RGBSpectrum, Spectrum>(move(mapping), fileName,
                                                       doTri, maxAniso,
                                                       (ImageWrap)wrapMode,
                                                       scale, gamma, doFilter);
    return ret;
}

REGISTER("Spectrumimage", createImageMap);

PALADIN_END
