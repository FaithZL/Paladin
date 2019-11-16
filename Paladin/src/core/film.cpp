//
//  film.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/8/20.
//

#include "film.hpp"
#include "tools/fileio.hpp"

PALADIN_BEGIN

Film::Film(const Point2i &resolution, const AABB2f &cropWindow,
           std::unique_ptr<Filter> filt, Float diagonal,
           const std::string &filename, Float scale,
           Float maxSampleLuminance):
fullResolution(resolution),
diagonal(diagonal * .001),
filter(std::move(filt)),
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
    std::lock_guard<std::mutex> lock(_mutex);
    for (Point2i pixel : tile->getPixelBounds()) {
        const FilmTilePixel &tilePixel = tile->getPixel(pixel);
        Pixel &mergePixel = getPixel(pixel);
        Float xyz[3];
        tilePixel.contribSum.ToXYZ(xyz);
        for (int i = 0; i < 3; ++i) {
            mergePixel.xyz[i] += xyz[i];
        }
        mergePixel.filterWeightSum += tilePixel.filterWeightSum;
    }
}

void Film::setImage(const Spectrum *img) const {
    int nPixels = croppedPixelBounds.area();
    for (int i = 0; i < nPixels; ++i) {
        Pixel &p = _pixels[i];
        img[i].ToXYZ(p.xyz);
        p.filterWeightSum = 1;
        p.splatXYZ[0] = p.splatXYZ[1] = p.splatXYZ[2] = 0;
    }
}

void Film::addSplat(const Point2f &p, Spectrum v) {
    if (!insideExclusive((Point2i)p, croppedPixelBounds)) {
        return;
    }
    if (v.y() > _maxSampleLuminance) {
        v *= _maxSampleLuminance / v.y();
    }
    Float xyz[3];
    v.ToXYZ(xyz);
    Pixel &pixel = getPixel((Point2i)p);
    for (int i = 0; i < 3; ++i) {
        pixel.splatXYZ[i].add(xyz[i]);
    }
}

void Film::writeImage(Float splatScale) {
    std::unique_ptr<Float[]> rgb(new Float[3 * croppedPixelBounds.area()]);
    int offset = 0;
    for (Point2i p : croppedPixelBounds) {
        // 将xyz转成rgb
        Pixel &pixel = getPixel(p);
        XYZToRGB(pixel.xyz, &rgb[3 * offset]);
        
        // I(x,y) = (∑f(x-xi,y-yi)w(xi,yi)L(xi,yi)) / (∑f(x-xi,y-yi))
        // 再列一遍过滤表达式
        Float filterWeightNum = pixel.filterWeightSum;
        if (filterWeightNum != 0) {
            Float invWeight = (Float)1 / filterWeightNum;
            rgb[3 * offset] = std::max((Float)0, rgb[3 * offset] * invWeight);
            rgb[3 * offset + 1] = std::max((Float)0, rgb[3 * offset + 1] * invWeight);
            rgb[3 * offset + 2] = std::max((Float)0, rgb[3 * offset + 2] * invWeight);
        }
        
        // 这里splat是双向方法用的，暂时不理
        Float splatRGB[3];
        Float splatXYZ[3] = {pixel.splatXYZ[0],
                            pixel.splatXYZ[1],
                            pixel.splatXYZ[2]};
        XYZToRGB(splatXYZ, splatRGB);

        rgb[3 * offset] += splatScale * splatRGB[0];
        rgb[3 * offset + 1] += splatScale * splatRGB[1];
        rgb[3 * offset + 2] += splatScale * splatRGB[2];
        
        rgb[3 * offset] *= _scale;
        rgb[3 * offset + 1] *= _scale;
        rgb[3 * offset + 2] *= _scale;
        ++offset;
    }
    // 最后保存文件，todo
    paladin::writeImage(filename, rgb.get(), croppedPixelBounds, fullResolution);
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

neb::CJsonObject Film::toJson() const {
    return nebJson();
}

//"param" : {
//    "resolution" : [400, 400],
//    "cropWindow" : null,
//    "fileName" : "conelbox.png",
//    "diagonal" : null,
//    "scale" : 1
//}
CObject_ptr createFilm(const nebJson &param, const Arguments &lst) {
    auto iter = lst.begin();
    
}

PALADIN_END



