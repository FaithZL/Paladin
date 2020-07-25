//
//  geometry.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/28.
//

#ifndef geometry_hpp
#define geometry_hpp

#include "core/integrator.hpp"
#include "core/shape.hpp"

PALADIN_BEGIN

enum class FieldType{
    GNormal,
    Tangent,
    UV,
    Depth,
    SNormal,
    Albedo,
    Distance,
    ShapeIdx,
    PrimIdx
};

class FieldIntegrator : public MonteCarloIntegrator {
    
public:
    FieldIntegrator(const std::shared_ptr<const Camera>& camera,const std::shared_ptr<Sampler>& sampler,const AABB2i& pixelBound,const string &type)
    : MonteCarloIntegrator(camera,sampler,pixelBound) {
        if (type == "snormal") {
            _type = FieldType::SNormal;
        } else if (type == "tangent") {
            _type = FieldType::Tangent;
        } else if (type == "uv") {
            _type = FieldType::UV;
        } else if (type == "depth") {
            _type = FieldType::Depth;
        } else if (type == "distance") {
            _type = FieldType::Distance;
        } else if (type == "gnormal") {
            _type = FieldType::GNormal;
        } else if (type == "albedo") {
            _type = FieldType::Albedo;
        } else if (type == "shapeidx") {
            _type = FieldType::ShapeIdx;
        } else if (type == "primidx") {
            _type = FieldType::PrimIdx;
        }
    }
    
    virtual nloJson toJson() const override {
        return nloJson();
    }
    
    virtual Spectrum Li(const RayDifferential &ray, const Scene &scene,
                                Sampler &sampler, MemoryArena &arena,
                                int depth = 0) const override{
        SurfaceInteraction ref;//和表面的交互点
        RGBSpectrum ret(0.0f);
        auto r = ray;
        if (scene.rayIntersect(ray, &ref)) {
            switch (_type) {
                case FieldType::SNormal: {
                    ref.computeTangentSpace();
                    auto mat = ref.shape->getMaterial();
                    if (mat) {
                        mat->computeScatteringFunctions(&ref, arena, TransportMode::Radiance, true);
                    }
                    Normal3f nn = normalize(ref.shading.normal);
                    nn = mapTo01(nn);
                    ret[0] = nn.x;
                    ret[1] = nn.y;
                    ret[2] = nn.z;
                }
                break;
                 
                case FieldType::Tangent: {
                    auto nn = normalize(ref.shading.dpdu);
                    //映射到[0~1]范围
                    nn = mapTo01(nn);
                    ret[0] = nn.x;
                    ret[1] = nn.y;
                    ret[2] = nn.z;
                }
                break;
                    
                default:
                    break;
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
    
    FieldType _type;
};

CObject_ptr createFieldIntegrator(const nloJson &param, const Arguments &lst);

PALADIN_END

#endif /* geometry_hpp */
