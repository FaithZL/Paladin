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
#include "core/camera.hpp"
#include "materials/bxdfs/bsdf.hpp"

PALADIN_BEGIN

enum class FieldType{
    GNormal,
    CPosition,
    Albedo,
    Tangent,
    Bitangent,
    UV,
    Depth,
    SNormal,
    Color,
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
        } else if (type == "bitangent") {
            _type = FieldType::Bitangent;
        } else if (type == "uv") {
            _type = FieldType::UV;
        } else if (type == "depth") {
            _type = FieldType::Depth;
        } else if (type == "distance") {
            _type = FieldType::Distance;
        } else if (type == "gnormal") {
            _type = FieldType::GNormal;
        } else if (type == "color") {
            _type = FieldType::Color;
        } else if (type == "albedo") {
            _type = FieldType::Albedo;
        } else if (type == "shapeidx") {
            _type = FieldType::ShapeIdx;
        } else if (type == "primidx") {
            _type = FieldType::PrimIdx;
        }
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
                    break;
                }
                case FieldType::Tangent: {
                    auto nn = normalize(ref.shading.dpdu);
                    //映射到[0~1]范围
                    nn = mapTo01(nn);
                    ret[0] = nn.x;
                    ret[1] = nn.y;
                    ret[2] = nn.z;
                    break;
                }
                case FieldType::Bitangent: {
                    auto nn = normalize(ref.shading.dpdv);
                    //映射到[0~1]范围
                    nn = mapTo01(nn);
                    ret[0] = nn.x;
                    ret[1] = nn.y;
                    ret[2] = nn.z;
                    break;
                }
                case FieldType::Distance: {
                    ret[0] = ret[1] = ret[2] = ray.tMax;
                    break;
                }
                case FieldType::ShapeIdx: {
                    ret[0] = ret[1] = ret[2] = ref.shapeIdx;
                    break;
                }
                case FieldType::PrimIdx: {
                    ret[0] = ret[1] = ret[2] = ref.primIdx;
                    break;
                }
                case FieldType::UV: {
                    ret[0] = ref.uv[0];
                    ret[1] = ref.uv[1];
                    ret[2] = 0;
                    break;
                }
                case FieldType::GNormal: {
                    auto nn = normalize(ref.normal);
                    //映射到[0~1]范围
                    nn = mapTo01(nn);
                    ret[0] = nn.x;
                    ret[1] = nn.y;
                    ret[2] = nn.z;
                    break;
                }
                case FieldType::CPosition: {
                    auto cPos = _camera->cameraToWorld.getInverse().exec(ref.time, ref.pos);
                    ret[0] = cPos[0];
                    ret[1] = cPos[1];
                    ret[2] = cPos[2];
                    break;
                }
                case FieldType::Depth: {
                    auto pCamera = static_cast<const ProjectiveCamera *>(_camera.get());
                    auto c2s = pCamera->getCameraToScreen();
                    auto cPos = _camera->cameraToWorld.getInverse().exec(ref.time, ref.pos);
                    auto sPos = c2s.exec(cPos);
                    ret[0] = ret[1] = ret[1] = sPos.z;
                    break;
                }
                case FieldType::Color: {
                    ret = ref.bsdf->getColor();
                    break;
                }
                case FieldType::Albedo: {
                    break;
                }
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
