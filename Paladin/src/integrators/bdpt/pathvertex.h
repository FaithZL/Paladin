//
//  pathvertex.h
//  Paladin
//
//  Created by SATAN_Z on 2019/12/31.
//

#ifndef pathvertex_h
#define pathvertex_h

#include "core/interaction.hpp"

PALADIN_BEGIN

static Float correctShadingNormal(const SurfaceInteraction &isect,
                            const Vector3f &wo, const Vector3f &wi,
                                  TransportMode mode) {
    // 推导过程详见bdpt.hpp注释
    if (mode == TransportMode::Importance) {
        Float num = absDot(wo, isect.shading.normal) * absDot(wi, isect.normal);
        Float denom = absDot(wo, isect.normal) * absDot(wi, isect.shading.normal);
        if (denom == 0) {
            return 0;
        }
        return num / denom;
    }
    return 1;
}

struct EndpointInteraction : public Interaction {
    union {
        const Camera *camera;
        const Light *light;
    };
    
    EndpointInteraction() : Interaction(), light(nullptr) {
        
    }
    
    EndpointInteraction(const Interaction &it, const Camera *camera)
    : Interaction(it),
    camera(camera) {
        
    }
    
    EndpointInteraction(const Camera *camera, const Ray &ray)
    : Interaction(ray.ori, ray.time, ray.medium), camera(camera) {
        
    }
    
    EndpointInteraction(const Light *light, const Ray &r, const Normal3f &nl)
    : Interaction(r.ori, r.time, r.medium), light(light) {
        normal = nl;
    }
    
    EndpointInteraction(const Interaction &it, const Light *light)
    : Interaction(it), light(light) {
        
    }
    
    EndpointInteraction(const Ray &ray)
    : Interaction(ray.at(1), ray.time, ray.medium), light(nullptr) {
        normal = Normal3f(-ray.dir);
    }
    
};

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
    Float pdfFwd = 0;
    Float pdfRev = 0;
    
    Vertex() : ei() {
        
    }
    
    Vertex(VertexType type, const EndpointInteraction &ei, const Spectrum &throughput)
    : type(type), throughput(throughput), ei(ei) {
        
    }
    
    Vertex(const SurfaceInteraction &si, const Spectrum &throughput)
    : type(VertexType::Surface), throughput(throughput), si(si) {
        
    }
    
    Vertex(const Vertex &v) {
        memcpy(this, &v, sizeof(Vertex));
    }
    
    Vertex &operator=(const Vertex &v) {
        memcpy(this, &v, sizeof(Vertex));
        return *this;
    }
    
    static Vertex createCamera(const Camera *camera, const Ray &ray,
                                const Spectrum &throughput);
    
    static Vertex createCamera(const Camera *camera, const Interaction &it,
                                      const Spectrum &throughput);
    
    static Vertex createLight(const Light *light, const Ray &ray,
                                     const Normal3f &nLight, const Spectrum &Le,
                                     Float pdf);
    
    static Vertex createLight(const EndpointInteraction &ei,
                                     const Spectrum &throughput, Float pdf);
    
    static Vertex createMedium(const MediumInteraction &mi,
                                      const Spectrum &throughput, Float pdf,
                                      const Vertex &prev);
    
    static Vertex createSurface(const SurfaceInteraction &si,
                                       const Spectrum &throughput, Float pdf,
                                       const Vertex &prev);
    
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
};

PALADIN_END

#endif /* pathvertex_h */
