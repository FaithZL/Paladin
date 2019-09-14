//
//  film.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/8/20.
//

#include "film.hpp"

PALADIN_BEGIN

Film::Film(const Point2i &resolution, const AABB2f &cropWindow,
           std::unique_ptr<Filter> filter, Float diagonal,
           const std::string &filename, Float scale,
           Float maxSampleLuminance):
fullResolution(resolution),
diagonal(diagonal * .001),
filter(std::move(filter)),
filename(filename),
_scale(scale),
_maxSampleLuminance(maxSampleLuminance) {
    // 计算渲染范围
    croppedPixelBounds = AABB2i(Point2i(std::ceil(fullResolution.x * cropWindow.pMin.x),
                     std::ceil(fullResolution.y * cropWindow.pMin.y)),
             Point2i(std::ceil(fullResolution.x * cropWindow.pMax.x),
                     std::ceil(fullResolution.y * cropWindow.pMax.y)));
    
    _pixels = std::unique_ptr<Pixel[]>(new Pixel[croppedPixelBounds.area()]);
    
    // 预先计算好filter函数的值
    // 只需要计算xy均大于0的情况
    int offset = 0;
    for (int y = 0; y < _filterTableWidth; ++y) {
        for (int x = 0; x < _filterTableWidth; ++x) {
            Point2f p;
            p.x = (x + 0.5f) * filter->radius.x / _filterTableWidth;
            p.y = (y + 0.5f) * filter->radius.y / _filterTableWidth;
            _filterTable[offset] = filter->evaluate(p);
            ++offset;
        }
    }
}

AABB2i Film::getSampleBounds() const {
    const Vector2f halfPixel = Vector2f(0.5f, 0.5f);
    Point2f pMin = floor(Point2f(croppedPixelBounds.pMin) + halfPixel - filter->radius);
    Point2f pMax = ceil(Point2f(croppedPixelBounds.pMax) - halfPixel + filter->radius);
    return AABB2i((Point2i)pMin, (Point2i)pMax);
}

AABB2f Film::getPhysicalExtent() const {
    Float aspect = (Float)fullResolution.y / (Float)fullResolution.x;
    Float x = std::sqrt(diagonal * diagonal / (1 + aspect * aspect));
    Float y = aspect * x;
    return AABB2f(Point2f(-x / 2, -y / 2), Point2f(x / 2, y / 2));
}

std::unique_ptr<FilmTile> Film::getFilmTile(const AABB2i &sampleBounds) {
    Vector2f halfPixel = Vector2f(0.5f, 0.5f);
    AABB2f floatBounds = (AABB2f)sampleBounds;
    Point2i p0 = (Point2i)ceil(floatBounds.pMin - halfPixel - filter->radius);
    Point2i p1 = (Point2i)floor(floatBounds.pMax - halfPixel + filter->radius) + Vector2i(1, 1);
    // 注意，这里需要与渲染的区域求交集
    AABB2i tilePixelBounds = intersect(AABB2i(p0, p1), croppedPixelBounds);
    FilmTile *pRet = new FilmTile(tilePixelBounds,
                                  filter->radius,
                                  _filterTable,
                                  _filterTableWidth,
                                  _maxSampleLuminance);
    return std::unique_ptr<FilmTile>(pRet);
}

void Film::mergeFilmTile(std::unique_ptr<FilmTile> tile) {
    
}

void Film::setImage(const Spectrum *img) const {
    
}

void Film::writeImage(Float splatScale) {
    int offset = 0;
    for (Point2i p : croppedPixelBounds) {
        
    }
}

void Film::clear() {
    for (Point2i p : croppedPixelBounds) {
        Pixel &pixel = getPixel(p);
        for (int i = 0; i < 3; ++i) {
            pixel.xyz[i] = pixel.splatXYZ[i] = 0;
        }
        pixel.filterWeightSum = 0;
    }
}

PALADIN_END



