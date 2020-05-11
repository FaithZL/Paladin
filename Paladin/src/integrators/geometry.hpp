//
//  geometry.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/28.
//

#ifndef geometry_hpp
#define geometry_hpp

#include "core/integrator.hpp"

PALADIN_BEGIN

enum class GeometryIntegratorType{
    Normal,Tangent
};

class GeometryIntegrator : public MonteCarloIntegrator {
    
public:
    GeometryIntegrator(const std::shared_ptr<const Camera>& camera,const std::shared_ptr<Sampler>& sampler,const AABB2i& pixelBound,const GeometryIntegratorType& type = GeometryIntegratorType::Normal)
    : MonteCarloIntegrator(camera,sampler,pixelBound),
    _type(type) {
        
    }
    
    virtual nloJson toJson() const override {
        return nloJson();
    }
    
    virtual Spectrum Li(const RayDifferential &ray, const Scene &scene,
                                Sampler &sampler, MemoryArena &arena,
                                int depth = 0) const override{
        SurfaceInteraction ref;//和表面的交互点
        RGBSpectrum ret(0.0f);
        
        if (scene.intersect(ray, &ref)) {
            if(_type == GeometryIntegratorType::Normal){
                ref.computeTangentSpace();
//                auto mat = ref.primitive->getMaterial();
//                if (mat) {
//                    mat->computeScatteringFunctions(&ref, arena, TransportMode::Radiance, true);
//                }
                Normal3f nn = normalize(ref.shading.normal);
                nn = mapTo01(nn);
                ret[0] = nn.x;
                ret[1] = nn.y;
                ret[2] = nn.z;
            } else if(_type == GeometryIntegratorType::Tangent){
                auto nn = normalize(ref.shading.dpdu);
                //映射到[0~1]范围
                nn = mapTo01(nn);
                ret[0] = nn.x;
                ret[1] = nn.y;
                ret[2] = nn.z;
            } else {
                auto nn = normalize(ref.shading.dpdv);
                //映射到[0~1]范围
                nn = mapTo01(nn);
                ret[0] = nn.x;
                ret[1] = nn.y;
                ret[2] = nn.z;
            }
            
            return ret;
        }
        return 0.0f;
    }
    
    inline Normal3f mapTo01(const Normal3f& n) const{
        return (n + Normal3f(1.0f, 1.0f, 1.0f)) * 0.5f;
    }

    inline Vector3f mapTo01(const Vector3f& n) const{
        return (n + Vector3f(1.0f, 1.0f, 1.0f)) * 0.5f;
    }
    
    
private:
    
    GeometryIntegratorType _type;
};

CObject_ptr createGeometryIntegrator(const nloJson &param, const Arguments &lst);

PALADIN_END

#endif /* geometry_hpp */
