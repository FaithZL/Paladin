//
//  imagemap.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/1.
//

#ifndef imagemap_hpp
#define imagemap_hpp

#include "core/texture.hpp"
#include "core/mipmap.h"

PALADIN_BEGIN

/**
 * 图片纹理
 * 两个模板类型，Tmemory为内存中的数据类型，Treturn为返回类型
 * 比如说内存中可以使用RGBSpectrum类，但返回值可以是Spectrum类
 * 假设我们使用的是SampledSpectrum编译，但内存中只需要储存RGB分量就可以了
 * 
 */
//template <typename Tmemory, typename Treturn>
//class ImageTexture : public Texture<Treturn> {
//
//public:
//    ImageTexture(std::unique_ptr<TextureMapping2D> m,
//                 const std::string &filename, bool doTri, Float maxAniso,
//                 ImageWrap wm, Float scale, bool gamma);
//
//    static void clearCache() {
//        _textures.erase(_textures.begin(), _textures.end());
//    }
//
//    virtual Treturn evaluate(const SurfaceInteraction &si) const {
//        Vector2f dstdx, dstdy;
//        Point2f st = _mapping->map(si, &dstdx, &dstdy);
//        Tmemory mem = mipmap->lookup(st, dstdx, dstdy);
//        Treturn ret;
//        convertOut(mem, &ret);
//        return ret;
//    }
//
//private:
//
//    std::unique_ptr<TextureMapping2D> _mapping;
//
//    MIPMap<Tmemory> *_mipmap;
//
//    static std::map<TexInfo, std::unique_ptr<MIPMap<Tmemory>>> _textures;
//};

PALADIN_END

#endif /* imagemap_hpp */
