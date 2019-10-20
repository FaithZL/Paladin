//
//  fileio.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "fileio.hpp"
#include "tools/fileutil.hpp"
#include "core/spectrum.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

PALADIN_BEGIN

RGBSpectrum * _readImage(const std::string &name,
                        int *width,
                        int *height) {
    unsigned char *rgb;
    int w, h;
    int channel;
    // 用stb库加载图片
    rgb = stbi_load(name.c_str(), &w, &h , &channel , 3);
    if (!rgb) {
        throw std::runtime_error(name + " load fail");
    }
    *width = w;
    *height = h;
    // 将rgb值转换为RGB光谱
    RGBSpectrum *ret = new RGBSpectrum[*width * *height];
    unsigned char *src = rgb;
    for (unsigned int y = 0; y < h; ++y) {
        for (unsigned int x = 0; x < w; ++x, src += 3) {
            Float c[3];
            c[0] = src[0] / 255.f;
            c[1] = src[1] / 255.f;
            c[2] = src[2] / 255.f;
            ret[y * *width + x] = RGBSpectrum::FromRGB(c);
        }
    }
    free(rgb);
    COUT << StringPrintf("Read image %s (%d x %d)", name.c_str(), *width, *height);
    return ret;
}

RGBSpectrum * _readImageHDR(const std::string &name, int *width, int *height) {
    int w,h;
    int comp;
    auto rgb = stbi_loadf(name.c_str(), &w, &h, &comp, 3);
    if (!rgb) {
        throw std::runtime_error(name + " load fail");
    }
    *width = w;
    *height = h;
    float *src = rgb;
    RGBSpectrum *ret = new RGBSpectrum[w * h];
    for (unsigned int y = 0; y < h; ++y) {
        for (unsigned int x = 0; x < w; ++x, src += 3) {
            Float c[3];
            c[0] = src[0] / 255.f;
            c[1] = src[1] / 255.f;
            c[2] = src[2] / 255.f;
            ret[y * *width + x] = RGBSpectrum::FromRGB(c);
        }
    }
    free(rgb);
    COUT << StringPrintf("Read HDR image %s (%d x %d)", name.c_str(), *width, *height);
    return ret;
}

std::unique_ptr<RGBSpectrum[]> readImage(const std::string &name, Point2i *resolution) {
    if (hasExtension(name, "hdr")) {
        return std::unique_ptr<RGBSpectrum []>(_readImageHDR(name, &resolution->x, &resolution->y));
    }
    // 暂时支持png，jpg，tga
    return std::unique_ptr<RGBSpectrum []>(_readImage(name, &resolution->x, &resolution->y));
}

void writeImage(const std::string &name,
                const Float *rgb,
                const AABB2i &outputBounds,
                const Point2i &totalResolution) {
    if (hasExtension(name, "hdr")) {
        Vector2i resolution = outputBounds.diagonal();
        std::unique_ptr<float[]> rgb24(new float[3 * resolution.x * resolution.y]);
        float *dst = rgb24.get();
        for (int y = 0; y < resolution.y; ++y) {
            for (int x = 0; x < resolution.x; ++x) {
#define TO_FLOAT(v) (float) (255.f * gammaCorrect(v))
                dst[0] = TO_FLOAT(rgb[3 * (y * resolution.x + x) + 0]);
                dst[1] = TO_FLOAT(rgb[3 * (y * resolution.x + x) + 1]);
                dst[2] = TO_FLOAT(rgb[3 * (y * resolution.x + x) + 2]);
#undef TO_FLOAT
                dst += 3;
            }
        }
        stbi_write_hdr(name.c_str(), resolution.x, resolution.y, 3, rgb24.get());
        return;
    }
    
    // 只有保存png，jpg，tga格式时，才需要clamp
    // 由于可能有clamp，可能导致hdr部分细节丢失
    // 如果需要保留hdr细节，直接保存hdr格式文件
    // 由于现在支持hdr的格式很多，直接保存就好了，所以就不实现tone mapping了
    Vector2i resolution = outputBounds.diagonal();
    std::unique_ptr<uint8_t[]> rgb8(new uint8_t[3 * resolution.x * resolution.y]);
    uint8_t *dst = rgb8.get();
    for (int y = 0; y < resolution.y; ++y) {
        for (int x = 0; x < resolution.x; ++x) {
#define TO_BYTE(v) (uint8_t) clamp(255.f * gammaCorrect(v), 0.f, 255.f)
            dst[0] = TO_BYTE(rgb[3 * (y * resolution.x + x) + 0]);
            dst[1] = TO_BYTE(rgb[3 * (y * resolution.x + x) + 1]);
            dst[2] = TO_BYTE(rgb[3 * (y * resolution.x + x) + 2]);
#undef TO_BYTE
            dst += 3;
        }
    }
    if (hasExtension(name, "png")) {
        stbi_write_png(name.c_str(), resolution.x, resolution.y, 3, rgb8.get(), 3 * resolution.x);
    } else if (hasExtension(name, "jpg")) {
        stbi_write_jpg(name.c_str(), resolution.x, resolution.y, 3, rgb8.get(), 100);
    } else if (hasExtension(name, "tga")) {
        stbi_write_tga(name.c_str(), resolution.x, resolution.y, 3, rgb8.get());
    }
}

PALADIN_END
