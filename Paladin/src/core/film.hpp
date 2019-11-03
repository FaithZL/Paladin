//
//  film.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/8/20.
//

#ifndef film_hpp
#define film_hpp

#include "core/header.h"
#include "core/spectrum.hpp"
#include "core/filter.h"
#include "tools/parallel.hpp"

PALADIN_BEGIN

/**
 * 用于储存像素的radiance值跟filter函数权重
 */
struct FilmTilePixel {
    Spectrum contribSum = 0.f;
    Float filterWeightSum = 0.f;
};

/**
 * 把整个胶片分为若干个块
 * 每个块就是一个FilmTile对象
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
        _pixels = std::vector<FilmTilePixel>(std::max(0, _pixelBounds.area()));
    }
    
    /**
     * 添加样本值，每次计算一次样本值的时候调用一次
     * @param pFilm        胶片像素上的点
     * @param L            radiance值
     * @param sampleWeight 采样权重(来自于相机)
     */
    void addSample(const Point2f &pFilm, Spectrum L, Float sampleWeight = 1.) {
        // todo 这里没有理解，为何要这样限制亮度，这样限制亮度会不会造成原有数据的改变？
        // 为何不是所有像素按照整体比例去缩放？
        if (L.y() > _maxSampleLuminance) {
            L *= _maxSampleLuminance / L.y();
        }

        // 找到受此样本影响范围内的像素
        Point2f pFilmDiscrete = pFilm - Vector2f(0.5f, 0.5f);
        Point2i p0 = (Point2i)ceil(pFilmDiscrete - _filterRadius);
        // 这里不知道为何向下取整再+1，为何不是直接向上取整todo
        Point2i p1 = (Point2i)floor(pFilmDiscrete + _filterRadius) + Point2i(1, 1);
        p0 = max(p0, _pixelBounds.pMin);
        p1 = min(p1, _pixelBounds.pMax);
        
        // todo这里有点奇怪，如果一个filter的xy方向上的过滤半径都为1，
        // 那么如果一个样本在落在一个像素内，那么这个样本点影响的像素个数应该是多少
        // 我觉得应该是3*3，九个像素，但pbrt代码上只影响两个像素，不符合预期
        
        int *offsetXList = ALLOCA(int, p1.x - p0.x);
        for (int x = p0.x; x < p1.x; ++x) {
            Float offsetX = std::abs((x - pFilmDiscrete.x) * _invFilterRadius.x *
                                _filterTableSize);
            offsetXList[x - p0.x] = std::min((int)std::floor(offsetX), _filterTableSize - 1);
        }
        int *offsetYList = ALLOCA(int, p1.y - p0.y);
        for (int y = p0.y; y < p1.y; ++y) {
            Float offsetY = std::abs((y - pFilmDiscrete.y) * _invFilterRadius.y * _filterTableSize);
            offsetYList[y - p0.y] = std::min((int)std::floor(offsetY), _filterTableSize - 1);
        }

        // I(x,y) = (∑f(x-xi,y-yi)w(xi,yi)L(xi,yi)) / (∑f(x-xi,y-yi))
        // 分别计算受该样本点影响的所有像素的滤波函数值并更新
        for (int y = p0.y; y < p1.y; ++y) {
            for (int x = p0.x; x < p1.x; ++x) {
                // 计算坐标点对应filter表的偏移量
                int offset = offsetYList[y - p0.y] * _filterTableSize + offsetXList[x - p0.x];
                // filterWeight = filter->evaluate(Point2i(x - pFilmDiscrete.x,
                //                             y - pFilmDiscrete.y));
                // 由于已经预计算好了，直接查表取值
                Float filterWeight = _filterTable[offset];
                
                // 更新像素值的贡献和以及过滤权重
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

/**
 * 可以理解为胶片类，相当于相机中接收光子 传感器
 * 将样本的贡献记录在film中有三种方式
 *     1.由采样器直接写入film中，但这种方式不适合并行，因为多个线程可能会同时操作一个像素
 * 因此，可以把film分为若干个tile，每个线程渲染一个tile，渲染一个tile的渲染之后，将像素值写入film中
 * 这样就可以很好的避免上述情况
 * 
 */
class Film {

public:
    
    Film(const Point2i &resolution, const AABB2f &cropWindow,
         std::unique_ptr<Filter> filter, Float diagonal,
         const std::string &filename, Float scale,
         Float maxSampleLuminance = Infinity);
    
    /**
     * 返回样本范围
     * @return [description]
     */
    AABB2i getSampleBounds() const;
    
    /**
     * 返回以(0,0)为中心的屏幕范围
     * @return [description]
     */
    AABB2f getPhysicalExtent() const;
    
    /**
     * 根据范围获取tile
     */
    std::unique_ptr<FilmTile> getFilmTile(const AABB2i &sampleBounds);
    
    void mergeFilmTile(std::unique_ptr<FilmTile> tile);
    
    void setImage(const Spectrum *img) const;
    
    /**
     * 双向方法中计算像素值的方式可能跟单向方法有所不同
     * 似乎不用filter这种加权的方式？还没看到双向方法，todo
     * 先把函数抄了再说，日后补上详解
     * @param p [description]
     * @param v [description]
     */
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
    
    // 把filter过滤的范围xy维度上各平均分割为_filterTableWidth个子区间
    // 在构造film的时候就预先计算好了filter范围内的每个子区间中点的值
    // 避免了在渲染过程中计算每个样本点的filter值的大计算量
    // 由过滤函数性质可知f(x,y) = f(|x|,|y|)
    // 所以只需要储存filter函数第一象限的值，
    // 也就是说只需要256的数组就能储存1024个filter函数的值
    // 这个精度基本很够用了
    Float _filterTable[_filterTableWidth * _filterTableWidth];

    // 互斥锁
    std::mutex _mutex; // 64字节

    // 双向方法用到的，暂时不知道什么东西
    const Float _scale;

    // 传感器能采样到的最大亮度
    const Float _maxSampleLuminance;
    
    Pixel &getPixel(const Point2i &p) {
        DCHECK(insideExclusive(p, croppedPixelBounds));
        int width = croppedPixelBounds.pMax.x - croppedPixelBounds.pMin.x;
        int offset = (p.x - croppedPixelBounds.pMin.x) + (p.y - croppedPixelBounds.pMin.y) * width;
        if (offset > 125000) {
            int a;
            a = 1;
        }
        return _pixels[offset];
    }
};


PALADIN_END

#endif /* film_hpp */
