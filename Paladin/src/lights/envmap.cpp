//
//  envmap.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/4.
//

#include "envmap.hpp"
#include "math/sampling.hpp"
#include "tools/fileio.hpp"

PALADIN_BEGIN

EnvironmentMap::EnvironmentMap(const Transform &LightToWorld,
                               const Spectrum &L,
                               int nSamples, const std::string &texmap)
:Light((int)LightFlags::Infinite, LightToWorld, MediumInterface(),
nSamples) {
    // 先初始化纹理贴图
    Point2i resolution;
    std::unique_ptr<RGBSpectrum[]> texels(nullptr);
    if (texmap != "") {
        texels = readImage(texmap, &resolution);
        if (texels) {
            for (int i = 0; i < resolution.x * resolution.y; ++i) {
                texels[i] = texels[i] * L.ToRGBSpectrum();
            }
        }
    }
    if (!texels) {
        resolution.x = 1;
        resolution.y = 1;
        texels = std::unique_ptr<RGBSpectrum[]>(new RGBSpectrum[1]);
        texels[0] = L.ToRGBSpectrum();
    }
    _Lmap.reset(new MIPMap<RGBSpectrum>(resolution, texels.get()));
    
    // 生成对应的二维分布
    int width = _Lmap->width();
    int height = _Lmap->height();
    std::unique_ptr<Float[]> img(new Float[width * height]);
    float fwidth = 0.5f / std::min(width, height);
    parallelFor([&](int64_t v) {
            Float vp = (v + 0.5f) / (Float)height;
            // 注意！需要计算sinθ，作为权重，因为平面映射到球面，θ越偏离90°，权重越低
            Float sinTheta = std::sin(Pi * (v + 0.5f) / height);
            for (int u = 0; u < width; ++u) {
                Float up = (u + 0.5f) / (Float)width;
                img[u + v * width] = _Lmap->lookup(Point2f(up, vp), fwidth).y();
                img[u + v * width] *= sinTheta;
            }
        },
        height, 32);
    _distribution.reset(new Distribution2D(img.get(), width, height));
}

Spectrum EnvironmentMap::power() const {
    // todo 球面映射会产生扭曲所以这样估计通量是有误差的
    return Pi * _worldRadius * _worldRadius *
           Spectrum(_Lmap->lookup(Point2f(.5f, .5f), .5f),
                    SpectrumType::Illuminant);
}

Spectrum EnvironmentMap::Le(const RayDifferential &ray) const {
    Vector3f w = normalize(_worldToLight.exec(ray.dir));
    Point2f st(sphericalPhi(w) * Inv2Pi, sphericalTheta(w) * InvPi);
    return Spectrum(_Lmap->lookup(st), SpectrumType::Illuminant);
}

Spectrum EnvironmentMap::sampleLi(const Interaction &ref, const Point2f &u,
                                      Vector3f *wi, Float *pdf,
                                      VisibilityTester *vis) const {
    Float mapPdf;
    Point2f uv = _distribution->sampleContinuous(u, &mapPdf);
    if (mapPdf == 0) {
        return Spectrum(0.f);
    }
    Float theta = uv[1] * Pi, phi = uv[0] * 2 * Pi;
    Float cosTheta = std::cos(theta), sinTheta = std::sin(theta);
    Float sinPhi = std::sin(phi), cosPhi = std::cos(phi);
    Vector3f wiLight = Vector3f(sinTheta * cosPhi,
                                sinTheta * sinPhi,
                                cosTheta);
    *wi = _lightToWorld.exec(wiLight);
    
    // p(u, v) / p(ω) = sinθ 2π^2
    *pdf = mapPdf / (2 * Pi * Pi * sinTheta);
    if (sinTheta == 0) {
        *pdf = 0;
    }

    *vis = VisibilityTester(ref, Interaction(ref.pos + *wi * (2 * _worldRadius),
                                             ref.time, mediumInterface));
    return Spectrum(_Lmap->lookup(uv), SpectrumType::Illuminant);
}

Float EnvironmentMap::pdfLi(const Interaction &, const Vector3f &w) const {
    Vector3f wi = _worldToLight.exec(w);
    Float theta = sphericalTheta(wi), phi = sphericalPhi(wi);
    Float sinTheta = std::sin(theta);
    if (sinTheta == 0) {
        return 0;
    }
    return _distribution->pdf(Point2f(phi * Inv2Pi, theta * InvPi)) /
           (2 * Pi * Pi * sinTheta);
}

PALADIN_END