//
//  film.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/8/20.
//

#ifndef film_hpp
#define film_hpp

#include "header.h"
#include "spectrum.hpp"
#include "filter.h"
#include "parallel.hpp"

PALADIN_BEGIN

struct FilmTilePixel {
    Spectrum contribSum = 0.f;
    Float filterWeightSum = 0.f;
};

/*
 把整个胶片分为若干个块
 */
class FilmTile {
public:
    
    FilmTile(const AABB2i &pixelBounds, const Vector2f &filterRadius,
             const Float *filterTable, int filterTableSize,
             Float maxSampleLuminance)
    : _pixelBounds(pixelBounds),
    _filterRadius(filterRadius),
    _invFilterRadius(1 / filterRadius.x, 1 / filterRadius.y),
    _filterTable(filterTable),
    _filterTableSize(filterTableSize),
    _maxSampleLuminance(maxSampleLuminance) {
        _pixels = std::vector<FilmTilePixel>(std::max(0, _pixelBounds.Area()));
    }
    
    void addSample(const Point2f &pFilm, Spectrum L,
                   Float sampleWeight = 1.) {

        if (L.y() > _maxSampleLuminance) {
            L *= _maxSampleLuminance / L.y();
        }
        Point2f pFilmDiscrete = pFilm - Vector2f(0.5f, 0.5f);
        Point2i p0 = (Point2i)ceil(pFilmDiscrete - _filterRadius);
        Point2i p1 = (Point2i)floor(pFilmDiscrete + _filterRadius) + Point2i(1, 1);
        p0 = max(p0, _pixelBounds.pMin);
        p1 = min(p1, _pixelBounds.pMax);
        
        int *ifx = ALLOCA(int, p1.x - p0.x);
        for (int x = p0.x; x < p1.x; ++x) {
            Float fx = std::abs((x - pFilmDiscrete.x) * _invFilterRadius.x *
                                _filterTableSize);
            ifx[x - p0.x] = std::min((int)std::floor(fx), _filterTableSize - 1);
        }
        int *ify = ALLOCA(int, p1.y - p0.y);
        for (int y = p0.y; y < p1.y; ++y) {
            Float fy = std::abs((y - pFilmDiscrete.y) * _invFilterRadius.y * _filterTableSize);
            ify[y - p0.y] = std::min((int)std::floor(fy), _filterTableSize - 1);
        }
        for (int y = p0.y; y < p1.y; ++y) {
            for (int x = p0.x; x < p1.x; ++x) {
                
                int offset = ify[y - p0.y] * _filterTableSize + ifx[x - p0.x];
                Float filterWeight = _filterTable[offset];
                
                FilmTilePixel &pixel = getPixel(Point2i(x, y));
                pixel.contribSum += L * sampleWeight * filterWeight;
                pixel.filterWeightSum += filterWeight;
            }
        }
    }
    
    FilmTilePixel &getPixel(const Point2i &p) {
        DCHECK(insideExclusive(p, _pixelBounds));
        int width = _pixelBounds.pMax.x - _pixelBounds.pMin.x;
        int offset = (p.x - _pixelBounds.pMin.x) + (p.y - _pixelBounds.pMin.y) * width;
        return _pixels[offset];
    }
    
    const FilmTilePixel &getPixel(const Point2i &p) const {
        DCHECK(insideExclusive(p, _pixelBounds));
        int width = _pixelBounds.pMax.x - _pixelBounds.pMin.x;
        int offset = (p.x - _pixelBounds.pMin.x) + (p.y - _pixelBounds.pMin.y) * width;
        return _pixels[offset];
    }
    
    AABB2i getPixelBounds() const {
        return _pixelBounds;
    }
    
private:
    // 像素的范围
    const AABB2i _pixelBounds;
    
    const Vector2f _filterRadius, _invFilterRadius;
    
    const Float * _filterTable;
    
    const int _filterTableSize;
    
    std::vector<FilmTilePixel> _pixels;
    
    const Float _maxSampleLuminance;
    
    friend class Film;
};

class Film {
public:
    
    Film(const Point2i &resolution, const AABB2f &cropWindow,
         std::unique_ptr<Filter> filter, Float diagonal,
         const std::string &filename, Float scale,
         Float maxSampleLuminance = Infinity);
    
    AABB2i getSampleBounds() const;
    
    AABB2f getPhysicalExtent() const;
    
    std::unique_ptr<FilmTile> getFilmTile(const AABB2i &sampleBounds);
    
    void mergeFilmTile(std::unique_ptr<FilmTile> tile);
    
    void setImage(const Spectrum *img) const;
    
    void addSplat(const Point2f &p, Spectrum v);
    
    void writeImage(Float splatScale = 1);
    
    void clear();
    
    // 图片分辨率，原点在左上角
    const Point2i fullResolution;

    // 对角线长度，单位为米
    const Float diagonal;

    // 过滤器
    std::unique_ptr<Filter> filter;

    // 文件名
    const std::string filename;

    // 需要渲染的子区域
    AABB2i croppedPixelBounds;
    
private:
    // 像素数据
    struct Pixel {
        Pixel() { 
            xyz[0] = xyz[1] = xyz[2] = filterWeightSum = 0; 
        }
        // xyz颜色空间，与设备无关
        Float xyz[3];
        // 所有样本的filter权重之和
        Float filterWeightSum;
        // 保存（未加权）样本splats总和
        AtomicFloat splatXYZ[3];
        // 占位，凑够32字节,为了避免一个对象横跨两个cache line，从而提高缓存命中率
        Float pad; 
    };
    
    // 像素列表
    std::unique_ptr<Pixel[]> _pixels;

    static CONSTEXPR int _filterTableWidth = 16;

    Float filterTable[_filterTableWidth * _filterTableWidth];

    std::mutex _mutex; // 64字节

    const Float _scale;

    const Float _maxSampleLuminance;
    
    Pixel &getPixel(const Point2i &p) {
        DCHECK(insideExclusive(p, croppedPixelBounds));
        int width = croppedPixelBounds.pMax.x - croppedPixelBounds.pMin.x;
        int offset = (p.x - croppedPixelBounds.pMin.x) + (p.y - croppedPixelBounds.pMin.y) * width;
        return _pixels[offset];
    }
};


PALADIN_END

#endif /* film_hpp */
