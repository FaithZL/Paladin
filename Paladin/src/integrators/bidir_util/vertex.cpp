//
//  vertex.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/2/7.
//

#include "vertex.hpp"
#include "endpoint.h"
#include "core/camera.hpp"
#include "core/light.hpp"
#include "core/primitive.hpp"
#include "core/scene.hpp"
#include "core/bxdf.hpp"
#include "func.hpp"
#include "materials/bxdfs/bsdf.hpp"

PALADIN_BEGIN

Vertex Vertex::createCamera(const Camera *camera, const Ray &ray,
                            const Spectrum &throughput) {
    return Vertex(VertexType::Camera, EndpointInteraction(camera, ray), throughput);
}

Vertex Vertex::createCamera(const Camera *camera, const Interaction &it,
                            const Spectrum &throughput) {
    return Vertex(VertexType::Camera, EndpointInteraction(it, camera), throughput);
}

Vertex Vertex::createLight(const Light *light, const Ray &ray,const Normal3f &nLight,
                           const Spectrum &Le,Float pdf) {
    Vertex ret(VertexType::Light, EndpointInteraction(light, ray, nLight), Le);
    ret.pdfFwd = pdf;
    return ret;
}

Vertex Vertex::createLight(const EndpointInteraction &ei,
                           const Spectrum &throughput, Float pdf) {
    auto ret = Vertex(VertexType::Light, ei, throughput);
    ret.pdfFwd = pdf;
    return ret;
}

Vertex Vertex::createMedium(const MediumInteraction &mi,
                            const Spectrum &throughput, Float pdf,
                            const Vertex &prev) {
    Vertex v(mi, throughput);
    v.pdfFwd = prev.convertPdf(pdf, v);
    return v;
}

Vertex Vertex::createSurface(const SurfaceInteraction &si,
                            const Spectrum &throughput, Float pdf,
                             const Vertex &prev) {
    Vertex v(si, throughput);
    v.pdfFwd = prev.convertPdf(pdf, v);
    return v;
}

Float Vertex::convertPdf(Float pdf, const Vertex &next) const {
    if (pdf == 0.f) {
        return 0.f;
    }
    // 如果下一个点在infinitylight上，直接返回
    // 因为相当于直接在单位半球上采样
    if (next.isInfiniteLight()) {
        return pdf;
    }
    Vector3f w = next.pos() - pos();
    if (w.lengthSquared() == 0) {
        return 0;
    }
    Float invDist2 = 1 / w.lengthSquared();
    if (next.isOnSurface()) {
        pdf *= absDot(next.ng(), w * std::sqrt(invDist2));
    }
    return pdf * invDist2;
}

const Interaction &Vertex::getInteraction() const {
    switch (type) {
        case VertexType::Medium:
            return mi;
        case VertexType::Surface:
            return si;
        default:
            return ei;
    }
}

Spectrum Vertex::f(const Vertex &next, TransportMode mode) const {
    Vector3f w = (next.pos() - pos());
    if (w.lengthSquared() == 0) {
        return 0.;
    }
    switch (type) {
        case VertexType::Medium:
            return mi.phase->p(mi.wo, w);
        case VertexType::Surface:
            return si.bsdf->f(si.wo, w) * correctShadingNormal(si, si.wo, w, mode);
        default:
            LOG(FATAL) << "Vertex::f(): Unimplemented";
            return Spectrum(0.f);
    }
}

const Point3f & Vertex::pos() const {
    return getInteraction().pos;
}

Float Vertex::time() const {
    return getInteraction().time;
}

const Normal3f &Vertex::ns() const {
    if (type == VertexType::Surface)
        return si.shading.normal;
    else
        return getInteraction().normal;
}

const Normal3f &Vertex::ng() const {
    return getInteraction().normal;
}

bool Vertex::isOnSurface() const {
    return !ng().isZero();
}

bool Vertex::isLight() const {
    return type == VertexType::Light ||
           (type == VertexType::Surface && si.primitive->getAreaLight());
}

bool Vertex::isDeltaLight() const {
    return type == VertexType::Light &&
            ei.light != nullptr &&
            ei.light->isDelta();
}

bool Vertex::isInfiniteLight() const {
    // InfiniteLight上的点的light指针为nullptr
    return type == VertexType::Light &&
           (!ei.light || ei.light->flags & (int)LightFlags::Infinite ||
            ei.light->flags & (int)LightFlags::DeltaDirection);
}

Spectrum Vertex::Le(const Scene &scene, const Vertex &ref) const {
    if (!isLight()) {
        return Spectrum(0.f);
    }
    Vector3f w = ref.pos() - pos();
    if (w.lengthSquared() == 0) {
        return Spectrum(0.f);
    }
    w = normalize(w);
    if (isInfiniteLight()) {
        Spectrum Le(0.f);
        for (const auto &light : scene.infiniteLights) {
            Le += light->Le(Ray(pos(), -w));
        }
        return Le;
    } else {
        const AreaLight *light = si.primitive->getAreaLight();
        CHECK(light != nullptr);
        return light->L(si, w);
    }
}

bool Vertex::isConnectible() const {
    switch (type) {
        case VertexType::Medium:
            return true;
        case VertexType::Light:
            return (ei.light->flags & (int)LightFlags::DeltaDirection) == 0;
        case VertexType::Camera:
            return true;
        case VertexType::Surface:
            // 如果没有non-specular的分量，则不能连接
            return si.bsdf->numComponents(BxDFType(BSDF_DIFFUSE | BSDF_GLOSSY |
                                               BSDF_REFLECTION |
                                               BSDF_TRANSMISSION)) > 0;
    }
    LOG(FATAL) << "Unhandled vertex type in IsConnectable()";
    return false;
}

Float Vertex::pdfDir(const Scene &scene, const Vertex *prev,
                     const Vertex &next) const {
    if (type == VertexType::Light) {
        return pdfLight(scene, next);
    }
    
    Vector3f wn = next.pos() - pos();
    if (wn.lengthSquared() == 0) {
        return 0;
    }
    wn = normalize(wn);
    Vector3f wp;
    if (prev) {
        wp = prev->pos() - pos();
        if (wp.lengthSquared() == 0) {
            return 0;
        }
        wp = normalize(wp);
    } else {
        CHECK(type == VertexType::Camera);
    }


    Float pdf = 0, unused;
    if (type == VertexType::Camera) {
        ei.camera->pdf_We(ei.spawnRay(wn), &unused, &pdf);
    } else if (type == VertexType::Surface) {
        pdf = si.bsdf->pdfDir(wp, wn);
    } else if (type == VertexType::Medium) {
        pdf = mi.phase->p(wp, wn);
    } else {
        LOG(FATAL) << "Vertex::Pdf(): Unimplemented";
    }
    return convertPdf(pdf, next);
}

Float Vertex::pdfLight(const Scene &scene, const Vertex &v) const {
    Vector3f w = v.pos() - pos();
    Float invDist2 = 1 / w.lengthSquared();
    w *= std::sqrt(invDist2);
    Float pdf;
    if (isInfiniteLight()) {
        Point3f worldCenter;
        Float worldRadius;
        scene.worldBound().boundingSphere(&worldCenter, &worldRadius);
        pdf = 1 / (Pi * worldRadius * worldRadius);
    } else {
        CHECK(isLight());
        const Light *light = type == VertexType::Light
                                 ? ei.light
                                 : si.primitive->getAreaLight();
        CHECK(light != nullptr);

        Float pdfPos, pdfDir;
        light->pdf_Le(Ray(pos(), w, Infinity, time()), ng(), &pdfPos, &pdfDir);
        pdf = pdfDir * invDist2;
    }
    if (v.isOnSurface()) {
        pdf *= absDot(v.ng(), w);
    }
    return pdf;
}

Float Vertex::pdfLightOrigin(const Scene &scene, const Vertex &v,
                             const Distribution1D &lightDistr,
                             const std::unordered_map<const Light *, size_t> &lightToDistrIndex) const {
    Vector3f w = v.pos() - pos();
    if (w.lengthSquared() == 0) return 0.;
    w = normalize(w);
    if (isInfiniteLight()) {
        return infiniteLightPdf(scene, lightDistr, lightToDistrIndex, w);
    } else {

        Float pdfPos, pdfDir, pdfChoice = 0;

        CHECK(isLight());
        const Light *light = type == VertexType::Light
                                 ? ei.light
                                 : si.primitive->getAreaLight();
        CHECK(light != nullptr);

        CHECK(lightToDistrIndex.find(light) != lightToDistrIndex.end());
        size_t index = lightToDistrIndex.find(light)->second;
        pdfChoice = lightDistr.discretePDF(index);
        // 采样到光源表面某个点的PDF，所以不需要pdfDir
        light->pdf_Le(Ray(pos(), w, Infinity, time()), ng(), &pdfPos, &pdfDir);
        return pdfPos * pdfChoice;
    }
}

PALADIN_END
