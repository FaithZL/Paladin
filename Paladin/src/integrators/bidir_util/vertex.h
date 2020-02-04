//
//  vertex.h
//  Paladin
//
//  Created by SATAN_Z on 2020/2/4.
//

#ifndef vertex_h
#define vertex_h

#include "endpoint.h"

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
        
    }
    
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
};

PALADIN_END

#endif /* vertex_h */
