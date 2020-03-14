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

//#include "ImfRgba.h"
//#include "ImfRgbaFile.h"

#include <ImfRgba.h>
#include <ImfRgbaFile.h>
#include "core/mipmap.h"


PALADIN_BEGIN

RGBSpectrum * _readImage(const std::string &name,
                        int *width,
                        int *height) {
    unsigned char *rgb;
    int w, h;
    int channel;
    // 用stb库加载图片
    rgb = stbi_load(name.c_str(), &w, &h , &channel , 4);
    if (!rgb) {
        throw std::runtime_error(name + " load fail");
    }
    *width = w;
    *height = h;
    // 将rgb值转换为RGB光谱
    RGBSpectrum *ret = new RGBSpectrum[*width * *height];
    unsigned char *src = rgb;
    for (unsigned int y = 0; y < h; ++y) {
        for (unsigned int x = 0; x < w; ++x, src += 4) {
            Float c[3];
            c[0] = src[0] / 255.f;
            c[1] = src[1] / 255.f;
            c[2] = src[2] / 255.f;
            if (channel == 4) {
                // todo
            }
            ret[y * *width + x] = RGBSpectrum::FromRGB(c);
        }
    }
    free(rgb);
    VLOG(2) << StringPrintf("Read image %s (%d x %d)", name.c_str(), *width, *height);
    return ret;
}


static void _writeImageEXR(const std::string &name, const Float *pixels,
                          int xRes, int yRes, int totalXRes, int totalYRes,
                          int xOffset, int yOffset) {
    using namespace Imf;
    using namespace Imath;

    Rgba *hrgba = new Rgba[xRes * yRes];
    for (int i = 0; i < xRes * yRes; ++i)
        hrgba[i] = Rgba(pixels[3 * i], pixels[3 * i + 1], pixels[3 * i + 2]);

    // OpenEXR uses inclusive pixel bounds.
    Box2i displayWindow(V2i(0, 0), V2i(totalXRes - 1, totalYRes - 1));
    Box2i dataWindow(V2i(xOffset, yOffset),
                     V2i(xOffset + xRes - 1, yOffset + yRes - 1));

    try {
        RgbaOutputFile file(name.c_str(), displayWindow, dataWindow,
                            WRITE_RGB);
        file.setFrameBuffer(hrgba - xOffset - yOffset * xRes, 1, xRes);
        file.writePixels(yRes);
    } catch (const std::exception &exc) {
        LOG(ERROR) << ("Error writing \"%s\": %s", name.c_str(), exc.what());
    }

    delete[] hrgba;
}


static RGBSpectrum * _readImageEXR(const std::string &name, int *width,
                          int *height, AABB2i *dataWindow = nullptr,
                          AABB2i *displayWindow = nullptr) {
    using namespace Imf;
    using namespace Imath;
    try {
        RgbaInputFile file(name.c_str());
        Box2i dw = file.dataWindow();

        if (dataWindow)
            *dataWindow = {{dw.min.x, dw.min.y}, {dw.max.x + 1, dw.max.y + 1}};
        if (displayWindow) {
            Box2i dispw = file.displayWindow();
            *displayWindow = {{dispw.min.x, dispw.min.y},
                              {dispw.max.x + 1, dispw.max.y + 1}};
        }
        *width = dw.max.x - dw.min.x + 1;
        *height = dw.max.y - dw.min.y + 1;

        std::vector<Rgba> pixels(*width * *height);
        file.setFrameBuffer(&pixels[0] - dw.min.x - dw.min.y * *width, 1,
                            *width);
        file.readPixels(dw.min.y, dw.max.y);
        
        RGBSpectrum *ret = new RGBSpectrum[*width * *height];
        for (int i = 0; i < *width * *height; ++i) {
            Float frgb[3] = {pixels[i].r, pixels[i].g, pixels[i].b};
            ret[i] = RGBSpectrum::FromRGB(frgb);
        }
        LOG(INFO) << StringPrintf("Read EXR image %s (%d x %d)",
                                  name.c_str(), *width, *height);
        return ret;
    } catch (const std::exception &e) {
        LOG(ERROR) << ("Unable to read image file \"%s\": %s", name.c_str(), e.what());
    }

    return NULL;
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
            c[0] = src[0];
            c[1] = src[1];
            c[2] = src[2] ;
            ret[y * *width + x] = RGBSpectrum::FromRGB(c);
        }
    }
    free(rgb);
    COUT << StringPrintf("Read HDR image %s (%d x %d)", name.c_str(), *width, *height);
    return ret;
}

inline int isWhitespace(char c) {
    return c == ' ' || c == '\n' || c == '\t';
}

int readWord(FILE *fp, char *buffer, int bufferLength) {
    int n;
    int c;

    if (bufferLength < 1) return -1;

    n = 0;
    c = fgetc(fp);
    while (c != EOF && !isWhitespace(c) && n < bufferLength) {
        buffer[n] = c;
        ++n;
        c = fgetc(fp);
    }

    if (n < bufferLength) {
        buffer[n] = '\0';
        return n;
    }

    return -1;
}

static constexpr bool hostLittleEndian = true;

#define BUFFER_SIZE 80

RGBSpectrum * _readImagePFM(const std::string &filename, int *xres, int *yres) {
    float *data = nullptr;
        RGBSpectrum *rgb = nullptr;
        char buffer[BUFFER_SIZE];
        unsigned int nFloats;
        int nChannels, width, height;
        float scale;
        bool fileLittleEndian;

        FILE *fp = fopen(filename.c_str(), "rb");
        if (!fp) goto fail;

        // read either "Pf" or "PF"
        if (readWord(fp, buffer, BUFFER_SIZE) == -1) goto fail;

        if (strcmp(buffer, "Pf") == 0)
            nChannels = 1;
        else if (strcmp(buffer, "PF") == 0)
            nChannels = 3;
        else
            goto fail;

        // read the rest of the header
        // read width
        if (readWord(fp, buffer, BUFFER_SIZE) == -1) goto fail;
        width = atoi(buffer);
        *xres = width;

        // read height
        if (readWord(fp, buffer, BUFFER_SIZE) == -1) goto fail;
        height = atoi(buffer);
        *yres = height;

        // read scale
        if (readWord(fp, buffer, BUFFER_SIZE) == -1) goto fail;
        sscanf(buffer, "%f", &scale);

        // read the data
        nFloats = nChannels * width * height;
        data = new float[nFloats];
        // Flip in Y, as P*M has the origin at the lower left.
        for (int y = height - 1; y >= 0; --y) {
            if (fread(&data[y * nChannels * width], sizeof(float),
                      nChannels * width, fp) != nChannels * width)
                goto fail;
        }

        // apply endian conversian and scale if appropriate
        fileLittleEndian = (scale < 0.f);
        if (hostLittleEndian ^ fileLittleEndian) {
            uint8_t bytes[4];
            for (unsigned int i = 0; i < nFloats; ++i) {
                memcpy(bytes, &data[i], 4);
                std::swap(bytes[0], bytes[3]);
                std::swap(bytes[1], bytes[2]);
                memcpy(&data[i], bytes, 4);
            }
        }
        if (std::abs(scale) != 1.f)
            for (unsigned int i = 0; i < nFloats; ++i) data[i] *= std::abs(scale);

        // create RGBs...
        rgb = new RGBSpectrum[width * height];
        if (nChannels == 1) {
            for (int i = 0; i < width * height; ++i) rgb[i] = RGBSpectrum(data[i]);
        } else {
            for (int i = 0; i < width * height; ++i) {
                Float frgb[3] = {data[3 * i], data[3 * i + 1], data[3 * i + 2]};
                rgb[i] = RGBSpectrum::FromRGB(frgb);
            }
        }

        delete[] data;
        fclose(fp);
        LOG(INFO) << StringPrintf("Read PFM image %s (%d x %d)",
                                  filename.c_str(), *xres, *yres);
        return rgb;

    fail:
        LOG(ERROR) << ("Error reading PFM file \"%s\"", filename.c_str());
        if (fp) fclose(fp);
        delete[] data;
        delete[] rgb;
        return nullptr;
}

std::unique_ptr<RGBSpectrum[]> readImage(const std::string &name, Point2i *resolution) {
    if (hasExtension(name, "hdr")) {
        return std::unique_ptr<RGBSpectrum []>(_readImageHDR(name, &resolution->x, &resolution->y));
    } else if (hasExtension(name, "exr")) {
        return std::unique_ptr<RGBSpectrum []>(_readImageEXR(name, &resolution->x, &resolution->y));
    } else if (hasExtension(name, "pfm")) {
        return std::unique_ptr<RGBSpectrum []>(_readImagePFM(name, &resolution->x, &resolution->y));
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
#define TO_FLOAT(v) (float) (gammaCorrect(v))
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
    } else if (hasExtension(name, "exr")) {
        _writeImageEXR(name, rgb, resolution.x, resolution.y, totalResolution.x,
                    totalResolution.y, outputBounds.pMin.x,
                    outputBounds.pMin.y);
    }
}

PALADIN_END
