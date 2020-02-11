//
//  vertex.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/2/7.
//

#ifndef vertex_hpp
#define vertex_hpp

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
    // 上个顶点正向采样当前顶点的pdf
    Float pdfFwd = 0;
    // 下个顶点反向采样到当前顶点的pdf（如果光反向传播）
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
    // 将基于面积的PDF转换为基于立体角的PDF
    Float convertPdf(Float pdf, const Vertex &next) const;
    
    const Interaction &getInteraction() const;
    
    Spectrum f(const Vertex &next, TransportMode mode) const;
    
    const Point3f &pos() const;
    
    Float time() const;
    
    const Normal3f &ng() const;
    
    const Normal3f &ns() const;
    
    bool isOnSurface() const;
    
    bool isLight() const;
    
    bool isDeltaLight() const;
    
    bool isInfiniteLight() const;
    
    Spectrum Le(const Scene &scene, const Vertex &v) const;
    
    bool isConnectible() const;
    
    Float pdfDir(const Scene &scene, const Vertex *prev,
                 const Vertex &next) const;
    
    Float pdfLight(const Scene &scene, const Vertex &v) const;
    
    Float pdfLightOrigin(const Scene &scene, const Vertex &v,
                         const Distribution1D &lightDistr,
                         const std::unordered_map<const Light *, size_t>
                         &lightToDistrIndex) const;
};

PALADIN_END

#endif /* vertex_hpp */
