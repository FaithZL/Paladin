//
//  vertex.h
//  Paladin
//
//  Created by SATAN_Z on 2020/2/4.
//

#ifndef vertex_h
#define vertex_h

#include "endpoint.h"
#include "core/camera.hpp"

PALADIN_BEGIN

enum class VertexType {
    Camera,
    Light,
    Surface,
    Medium
};

struct Vertex {
    VertexType type;
    Spectrum throughput;
    
    struct {
        EndpointInteraction ei;
        MediumInteraction mi;
        SurfaceInteraction si;
    };
    
    bool delta = false;
    // 当前顶点的前向pdf
    Float pdfFwd = 0;
    // 当前顶点的后向pdf（如果光反向传播）
    Float pdfRev = 0;
    
    Vertex() : ei() {
        
    }
    
    Vertex(VertexType type, const EndpointInteraction &ei, const Spectrum &throughput)
    : type(type), throughput(throughput), ei(ei) {
        
    }
    
    Vertex(const SurfaceInteraction &si, const Spectrum &throughput)
    : type(VertexType::Surface), throughput(throughput), si(si) {
        
    }
    
    Vertex(const MediumInteraction &mi, const Spectrum &throughput)
    : type(VertexType::Medium), throughput(throughput), mi(mi) {
        
    }
    
    Vertex(const Vertex &v) {
        memcpy(this, &v, sizeof(Vertex));
    }
    
    Vertex &operator=(const Vertex &v) {
        memcpy(this, &v, sizeof(Vertex));
        return *this;
    }
    
    static Vertex createCamera(const Camera *camera, const Ray &ray,
                               const Spectrum &throughput) {
        return Vertex(VertexType::Camera, EndpointInteraction(camera, ray), throughput);
    }
    
    static Vertex createCamera(const Camera *camera, const Interaction &it,
                               const Spectrum &throughput) {
        return Vertex(VertexType::Camera, EndpointInteraction(it, camera), throughput);
    }
    
    static Vertex createLight(const Light *light, const Ray &ray,
                                     const Normal3f &nLight, const Spectrum &Le,
                              Float pdf) {
        return Vertex(VertexType::Light, EndpointInteraction(light, ray, nLight), Le);
    }
    
    static Vertex createLight(const EndpointInteraction &ei,
                              const Spectrum &throughput, Float pdf) {
        auto ret = Vertex(VertexType::Light, ei, throughput);
        ret.pdfFwd = pdf;
        return ret;
    }
    
    static Vertex createMedium(const MediumInteraction &mi,
                                      const Spectrum &throughput, Float pdf,
                               const Vertex &prev) {
        Vertex v(mi, throughput);
        v.pdfFwd = prev.convertDensity(pdf, v);
        return v;
    }
    
    static Vertex createSurface(const SurfaceInteraction &si,
                                       const Spectrum &throughput, Float pdf,
                                       const Vertex &prev);
    // 将基于面积的PDF转换为基于立体角的PDF
    Float convertDensity(Float pdf, const Vertex &next) const {
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
    
    const Interaction &getInteraction() const {
        switch (type) {
            case VertexType::Medium:
                return mi;
            case VertexType::Surface:
                return si;
            default:
                return ei;
        }
    }
    
    const Point3f &pos() const {
        return getInteraction().pos;
    }
    
    Float time() const {
        return getInteraction().time;
    }
    
    const Normal3f &ng() const {
        return getInteraction().normal;
    }
    
    const Normal3f &ns() const {
        if (type == VertexType::Surface)
            return si.shading.normal;
        else
            return getInteraction().normal;
    }
    
    bool isOnSurface() const {
        return !ng().isZero();
    }
    
    bool isLight() const {
        return type == VertexType::Light ||
               (type == VertexType::Surface && si.primitive->getAreaLight());
    }
    
    bool isDeltaLight() const {
        return type == VertexType::Light &&
                ei.light != nullptr &&
                ei.light->isDelta();
    }
    
    bool isInfiniteLight() const {
        return type == VertexType::Light &&
               (!ei.light || ei.light->flags & (int)LightFlags::Infinite ||
                ei.light->flags & (int)LightFlags::DeltaDirection);
    }
    
    Spectrum Le(const Scene &scene, const Vertex &v) const {
        if (!isLight()) {
            return Spectrum(0.f);
        }
        Vector3f w = v.pos() - pos();
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
    
    bool isConnectible() const {
        switch (type) {
        case VertexType::Medium:
            return true;
        case VertexType::Light:
            return (ei.light->flags & (int)LightFlags::DeltaDirection) == 0;
        case VertexType::Camera:
            return true;
        case VertexType::Surface:
            return si.bsdf->numComponents(BxDFType(BSDF_DIFFUSE | BSDF_GLOSSY |
                                                   BSDF_REFLECTION |
                                                   BSDF_TRANSMISSION)) > 0;
        }
        LOG(FATAL) << "Unhandled vertex type in IsConnectable()";
        return false;
    }
    
    Float pdfDir(const Scene &scene, const Vertex *prev,
              const Vertex &next) const {
        if (type == VertexType::Light) {
            return pdfLight(scene, next);
        }
        
        Vector3f wn = next.pos() - pos();
        if (wn.lengthSquared() == 0) return 0;
        wn = normalize(wn);
        Vector3f wp;
        if (prev) {
            wp = prev->pos() - pos();
            if (wp.lengthSquared() == 0) return 0;
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
        return convertDensity(pdf, next);
    }
    
    Float pdfLight(const Scene &scene, const Vertex &v) const {
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
};

PALADIN_END

#endif /* vertex_h */
