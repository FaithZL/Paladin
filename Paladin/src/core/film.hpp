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
    
    void AddSample(const Point2f &pFilm, Spectrum L,
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
    AABB2i GetSampleBounds() const;
    AABB2f GetPhysicalExtent() const;
    std::unique_ptr<FilmTile> GetFilmTile(const AABB2i &sampleBounds);
    void MergeFilmTile(std::unique_ptr<FilmTile> tile);
    void SetImage(const Spectrum *img) const;
    void AddSplat(const Point2f &p, Spectrum v);
    void WriteImage(Float splatScale = 1);
    void Clear();
    
    // Film Public Data
    const Point2i fullResolution;
    const Float diagonal;
    std::unique_ptr<Filter> filter;
    const std::string filename;
    AABB2i croppedPixelBounds;
    
private:
    // Film Private Data
    struct Pixel {
        Pixel() { xyz[0] = xyz[1] = xyz[2] = filterWeightSum = 0; }
        Float xyz[3];
        Float filterWeightSum;
        AtomicFloat splatXYZ[3];
        Float pad;
    };
    std::unique_ptr<Pixel[]> _pixels;
    static CONSTEXPR int _filterTableWidth = 16;
    Float filterTable[_filterTableWidth * _filterTableWidth];
    std::mutex _mutex;
    const Float _scale;
    const Float _maxSampleLuminance;
    
    // Film Private Methods
    Pixel &GetPixel(const Point2i &p) {
        DCHECK(insideExclusive(p, croppedPixelBounds));
        int width = croppedPixelBounds.pMax.x - croppedPixelBounds.pMin.x;
        int offset = (p.x - croppedPixelBounds.pMin.x) +
        (p.y - croppedPixelBounds.pMin.y) * width;
        return _pixels[offset];
    }
};


PALADIN_END

#endif /* film_hpp */
