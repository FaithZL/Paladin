//
//  diffuse.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/18.
//

#include "diffuse.hpp"
#include "core/shape.hpp"
#include "math/sampling.hpp"
#include "core/bxdf.hpp"
#include "core/scene.hpp"

PALADIN_BEGIN

DiffuseAreaLight::DiffuseAreaLight(const Transform * LightToWorld,
                                   const MediumInterface &mediumInterface,
                                   const Spectrum &L, int nSamples,
                                   const std::shared_ptr<Shape> &shape,
                                   bool twoSided,
                                   const string &texname)
: AreaLight(LightToWorld, mediumInterface, nSamples),
_L(L),
_shape(shape),
_twoSided(twoSided),
_area(_shape->area()),
_Lmap(nullptr) {
    if (!texname.empty()) {
        loadLeMap(texname);
    }
}

void DiffuseAreaLight::loadLeMap(const string &texname) {
    _Lmap = ImageTexture<RGBSpectrum, Spectrum>::getTexture(texname, true, 8, ImageWrap::Repeat, 1, false);
}

Spectrum DiffuseAreaLight::power() const {
    return (_twoSided ? _2Pi : Pi) * _L * _area;
}

Spectrum DiffuseAreaLight::sample_Li(const Interaction &ref, const Point2f &u,
                                     Vector3f *wi, Float *pdf,
                                     VisibilityTester *vis) const {
    TRY_PROFILE(Prof::LightSample)
    Interaction pShape = _shape->sampleDir(ref, u, pdf);
    pShape.mediumInterface = mediumInterface;
    if (*pdf == 0 || (pShape.pos - ref.pos).lengthSquared() == 0) {
        *pdf = 0;
        return 0.f;
    }
    *wi = normalize(pShape.pos - ref.pos);
    *vis = VisibilityTester(ref, pShape);
    return L(pShape, -*wi);
}

Spectrum DiffuseAreaLight::sample_Li(DirectSamplingRecord *rcd, const Point2f &u,
                                     const Scene &scene) const {
    TRY_PROFILE(Prof::LightSample)
    _shape->sampleDir(rcd, u);
    if (rcd->pdfDir() == 0) {
        return 0;
    }
    Spectrum ret = L(*rcd);
    auto vis = rcd->getVisibilityTester();
    ret = (!ret.IsBlack() && vis.unoccluded(scene)) ? ret : Spectrum(0.f);
    return ret;
}

Float DiffuseAreaLight::pdf_Li(const Interaction &ref,
                               const Vector3f &wi) const {
    TRY_PROFILE(Prof::LightPdf)
    return _shape->pdfDir(ref, wi);
}

Float DiffuseAreaLight::pdf_Li(const DirectSamplingRecord &rcd) const {
    TRY_PROFILE(Prof::LightPdf)
    return rcd.pdfDir();
}

Spectrum DiffuseAreaLight::sample_Le(const Point2f &u1, const Point2f &u2,
                                     Float time, Ray *ray, Normal3f *nLight,
                                     Float *pdfPos, Float *pdfDir) const {
    auto lightIntr = _shape->samplePos(u1, pdfPos);
    lightIntr.mediumInterface = mediumInterface;
    *nLight = lightIntr.normal;
    
    Vector3f w;
    if (_twoSided) {
        Point2f u = u2;
        if (u[0] < 0.5f) {
            u[0] = std::min(u[0] * 2, OneMinusEpsilon);
            w = cosineSampleHemisphere(u);
        } else {
            u[0] = std::min(2 * u[0] - 1, OneMinusEpsilon);
            w = cosineSampleHemisphere(u);
            w.z *= -1;
        }
        *pdfDir = 0.5 * cosineHemispherePdf(Frame::cosTheta(w));
    } else {
        w = cosineSampleHemisphere(u2);
        *pdfDir = cosineHemispherePdf(Frame::cosTheta(w));
    }
    
    Frame frame(lightIntr.normal);
    // 将w转到世界空间
    w = frame.toWorld(w);
    *ray = lightIntr.spawnRay(w);
    return L(lightIntr, w);
}

void DiffuseAreaLight::pdf_Le(const Ray &ray, const Normal3f &nLight,
                              Float *pdfPos, Float *pdfDir) const {
    *pdfPos = _shape->pdfPos();
    *pdfDir = _twoSided ?
            0.5f * cosineHemispherePdf(absDot(nLight, ray.dir)) :
            cosineHemispherePdf(dot(nLight, ray.dir));
}

shared_ptr<DiffuseAreaLight> DiffuseAreaLight::create(Float rgb[3], const std::shared_ptr<Shape> &shape,
                                                      const MediumInterface &mi,const string &texname) {
    if (rgb[0] == 0 && rgb[1] == 0 && rgb[2] == 0) {
        return nullptr;
    }
    Spectrum Le = Spectrum::FromRGB(rgb);
    auto l2w = shape->objectToWorld;
    return make_shared<DiffuseAreaLight>(l2w, mi, Le, 1, shape, false);
}

//"emission" : {
//    "nSamples" : 1,
//    "Le" : {
//        "colorType" : 1,
//        "color" : [1,1,1],
//    },
//    "scale" : 1.f,
//    "twoSided" : false
//}
DiffuseAreaLight * createDiffuseAreaLight(const nloJson &param,
                                          const std::shared_ptr<Shape> &shape,
                                          const MediumInterface &mi) {
    if (param.is_null()) {
        return nullptr;
    }
    auto l2w = shape->objectToWorld;
    nloJson _Le = param.value("Le", nloJson::object());
    nloJson scale = param.value("scale", 1.f);
    Spectrum Le = Spectrum::FromJson(_Le);
    Le *= (Float)scale;
    bool twoSided = param.value("twoSided", false);
    int nSamples = param.value("nSamples", 1);
    return new DiffuseAreaLight(l2w, mi, Le, nSamples, shape, twoSided);
}
PALADIN_END
