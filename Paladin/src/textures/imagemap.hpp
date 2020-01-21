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
#include "tools/fileio.hpp"

PALADIN_BEGIN

/**
 * 纹理信息
 * todo，这里可以进行内存方面的优化
 * 如果仅仅是doTrilinear，或maxAniso这类属性方面的差异，是可以复用的，不需要再创建
 */
struct TexInfo {
    TexInfo(const std::string &f, bool dt, Float ma, ImageWrap wm, Float sc, bool gamma)
    : filename(f),
	doTrilinear(dt),
	maxAniso(ma),
	wrapMode(wm),
	scale(sc),
	gamma(gamma) {

	}
	// 文件名
    std::string filename;
    // 是否进行三角过滤
    bool doTrilinear;
    // 各向异性最大比例
    Float maxAniso;
    // 环绕模式
    ImageWrap wrapMode;
    // 缩放比例
    Float scale;
    // 是否需要伽马校正
    bool gamma;
    bool operator<(const TexInfo &t2) const {
        if (filename != t2.filename) 
        	return filename < t2.filename;
        if (doTrilinear != t2.doTrilinear) 
        	return doTrilinear < t2.doTrilinear;
        if (maxAniso != t2.maxAniso) 
        	return maxAniso < t2.maxAniso;
        if (scale != t2.scale) 
        	return scale < t2.scale;
        if (gamma != t2.gamma) 
        	return !gamma;
        return wrapMode < t2.wrapMode;
    }
};

/**
 * 图片纹理
 * 两个模板类型，Tmemory为内存中的数据类型，Treturn为返回类型
 * 比如说内存中可以使用RGBSpectrum类，但返回值可以是Spectrum类
 * 假设我们使用的是SampledSpectrum编译，但内存中只需要储存RGB分量就可以了
 * 
 */
template <typename Tmemory, typename Treturn>
class ImageTexture : public Texture<Treturn> {

public:
	ImageTexture(std::unique_ptr<TextureMapping2D> mapping,
	            const std::string &filename, bool doTri, Float maxAniso,
	            ImageWrap wm, Float scale, bool gamma)
	: _mapping(std::move(mapping)) {
		_mipmap = getTexture(filename, doTri, maxAniso, wm, scale, gamma);
	}

	static void clearCache() {
	   _imageCache.erase(_imageCache.begin(), _imageCache.end());
	}

	virtual Treturn evaluate(const SurfaceInteraction &si) const override {
	   Vector2f dstdx, dstdy;
	   Point2f st = _mapping->map(si, &dstdx, &dstdy);
	   Tmemory mem = _mipmap->lookup(st, dstdx, dstdy);
	   Treturn ret;
	   convertOut(mem, &ret);
	   return ret;
	}
            
    virtual nloJson toJson() const override {
        return nloJson();
    }
            
    static MIPMap<Tmemory> *getTexture(const std::string &filename,
                                       bool doTrilinear,
                                       Float maxAniso,
                                       ImageWrap wm,
                                       Float scale,
                                       bool gamma) {
        TexInfo textInfo(filename, doTrilinear, maxAniso, wm, scale, gamma);
        // 先从纹理缓存中查找，如果找得到，直接返回对应mipmap指针
        if (_imageCache.find(textInfo) != _imageCache.end()) {
            return _imageCache[textInfo].get();
        }
        Point2i resolution;
        std::unique_ptr<RGBSpectrum[]> texels = readImage(filename, &resolution);
        if (!texels) {
            // 如果图片读取失败，则创建常量纹理
            resolution.x = resolution.y = 1;
            RGBSpectrum *rgb = new RGBSpectrum[1];
            *rgb = RGBSpectrum(0.5f) * scale;
            texels.reset(rgb);
        }
        // 图片保存在内存中左上角为原点
        // 纹理坐标系中左下角为原点，需要转换一下
        for (int y = 0; y < resolution.y / 2; ++y) {
            for (int x = 0; x < resolution.x; ++x) {
                int o1 = y * resolution.x + x;
                int o2 = (resolution.y - y - 1) * resolution.x + x;
                std::swap(texels[o1], texels[o2]);
            }
        }

        MIPMap<Tmemory> *mipmap = nullptr;
        std::unique_ptr<Tmemory[]> convertedTexels(new Tmemory[resolution.x *
                                                               resolution.y]);
        for (int i = 0; i < resolution.x * resolution.y; ++i) {
            convertIn(texels[i], &convertedTexels[i], scale, gamma);
        }
        mipmap = new MIPMap<Tmemory>(resolution, convertedTexels.get(),
                                     doTrilinear, maxAniso, wm);
        _imageCache[textInfo].reset(mipmap);
        return mipmap;
    }

private:

	static void convertIn(const RGBSpectrum &from, RGBSpectrum *to, Float scale, bool gamma) {
		for (int i = 0; i < RGBSpectrum::nSamples; ++i) {
			(*to)[i] = scale * (gamma ? inverseGammaCorrect(from[i]) : from[i]);
		}
	}

	static void convertIn(const RGBSpectrum &from, Float *to, Float scale, bool gamma) {
        *to = scale * (gamma ? inverseGammaCorrect(from.y()) : from.y());
    }

	static void convertOut(const RGBSpectrum &from, Spectrum *to) {
        Float rgb[3];
        from.ToRGB(rgb);
        *to = Spectrum::FromRGB(rgb);
    }

    static void convertOut(Float from, Float *to) { 
    	*to = from; 
    }

	// 纹理映射方式
	std::unique_ptr<TextureMapping2D> _mapping;

	MIPMap<Tmemory> *_mipmap;

	static std::map<TexInfo, std::unique_ptr<MIPMap<Tmemory>>> _imageCache;
};


shared_ptr<ImageTexture<RGBSpectrum, Spectrum>> createImageMap(const string &filename, bool gamma = false, bool doTri = true,
                                        Float maxAniso = 8, ImageWrap wm = ImageWrap::Repeat, Float scale = 1,
                                        unique_ptr<TextureMapping2D> mapping = unique_ptr<TextureMapping2D>(new UVMapping2D()));
            
CObject_ptr createImageMap(const nloJson &param, const Arguments &lst);
            
PALADIN_END

#endif /* imagemap_hpp */
