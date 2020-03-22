//
//  imagemap.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/1.
//

#include "imagemap.hpp"
#include "core/paladin.hpp"

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

shared_ptr<ImageTexture<Float, Float>> createFloatMap(const string &filename, bool gamma, bool doTri,
                                                            Float maxAniso, ImageWrap wm, Float scale,
                                                            bool doFilter,
                                                            unique_ptr<TextureMapping2D> mapping) {
    return make_shared<ImageTexture<Float, Float>>(move(mapping),
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
//    "fromBasePath" : false,
//    "gamma" : false
//}
CObject_ptr createImageMap(const nloJson &param, const Arguments &lst) {
    auto mapping = unique_ptr<TextureMapping2D>(new UVMapping2D());
    string fn = param.value("fileName", "");
    bool fromBasePath = param.value("fromBasePath", false);
    if (fromBasePath) {
        string basePath = Paladin::getInstance()->getBasePath();
        fn = basePath + fn;
    }
    bool doTri = param.value("doTri", true);
    Float maxAniso = param.value("maxAniso", 8.f);
    int wrapMode = param.value("wrapMode", 0);
    Float scale = param.value("scale", 1.f);
    bool gamma = param.value("gamma", false);
    bool doFilter = param.value("doFilter", true);
    auto ret = new ImageTexture<RGBSpectrum, Spectrum>(move(mapping), fn,
                                                       doTri, maxAniso,
                                                       (ImageWrap)wrapMode,
                                                       scale, gamma, doFilter);
    return ret;
}

//"param" : {
//    "fileName" : "res/planet_Quom1200.png",
//    "doTri" : true,
//    "maxAniso" : 8,
//    "wrapMode" : 0,
//    "scale" : 1,
//    "gamma" : false
//}
CObject_ptr createFloatMap(const nloJson &param, const Arguments &lst) {
    auto mapping = unique_ptr<TextureMapping2D>(new UVMapping2D());
    string fn = param.value("fileName", "");
    bool fromBasePath = param.value("fromBasePath", false);
    if (fromBasePath) {
        string basePath = Paladin::getInstance()->getBasePath();
        fn = basePath + fn;
    }
    bool doTri = param.value("doTri", true);
    Float maxAniso = param.value("maxAniso", 8.f);
    int wrapMode = param.value("wrapMode", 0);
    Float scale = param.value("scale", 1.f);
    bool gamma = param.value("gamma", false);
    bool doFilter = param.value("doFilter", true);
    auto ret = new ImageTexture<Float, Float>(move(mapping), fn,
                                                       doTri, maxAniso,
                                                       (ImageWrap)wrapMode,
                                                       scale, gamma, doFilter);
    return ret;
}

REGISTER("Spectrumimage", createImageMap);
REGISTER("Floatimage", createFloatMap);

PALADIN_END
