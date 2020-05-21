//
//  primitive.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/8/10.
//

#ifndef primitive_hpp
#define primitive_hpp

#include "core/header.h"
#include "core/interaction.hpp"
#include "core/material.hpp"
#include "math/animatedtransform.hpp"
#include "core/cobject.h"
#include "tools/embree_util.hpp"

PALADIN_BEGIN

// Primitive可以理解为片段
class Primitive : public CObject {
public:
    virtual ~Primitive() {
        
    }
    
    virtual AABB3f worldBound() const = 0;
    
    virtual bool intersect(const Ray &r, SurfaceInteraction *) const {
        DCHECK(false);
    }
    
    virtual bool intersectP(const Ray &r) const{
        DCHECK(false);
    }
    
    
    virtual bool rayIntersect(const Ray &ray, SurfaceInteraction * isect) const {
        DCHECK(false);
    }
    
    virtual bool rayOccluded(const Ray &ray) const {
        DCHECK(false);
    }
    
    virtual EmbreeUtil::EmbreeGeomtry * getEmbreeGeometry() const {
        return nullptr;
    }
    
    virtual bool fillSurfaceInteraction(const Ray &r, const Vector2f &uv, SurfaceInteraction *isect) const {
        DCHECK(false);
        return true;
    }
    
    virtual const AreaLight *getAreaLight() const = 0;
    
    virtual const Material *getMaterial() const = 0;
    
    virtual RTCGeometry rtcGeometry(Scene * scene) const {
        return nullptr;
    }
    
    virtual void computeScatteringFunctions(SurfaceInteraction *isect,
                                            MemoryArena &arena,
                                            TransportMode mode,
                                            bool allowMultipleLobes) const = 0;
};

//// 几何片元，有形状，材质，是否发光等属性，是需要渲染的具体物件
//class GeometricPrimitive : public Primitive {
//    
//public:
//    
//    GeometricPrimitive(const std::shared_ptr<Shape> &shape,
//                          const std::shared_ptr<const Material> &material,
//                          const std::shared_ptr<AreaLight> &areaLight,
//                          const MediumInterface &mediumInterface);
//    
//    virtual AABB3f worldBound() const override;
//    
//    virtual AABB3f objectBound() const;
//    
//    virtual bool intersect(const Ray &r, SurfaceInteraction *isect) const override;
//    
//    virtual bool intersectP(const Ray &r) const override;
//    
//    virtual const AreaLight *getAreaLight() const override;
//    
//    virtual const Material *getMaterial() const override;
//    
//    virtual RTCGeometry rtcGeometry(Scene * scene) const override;
//    
//    virtual EmbreeUtil::EmbreeGeomtry * getEmbreeGeometry() const override;
//    
//    // 用于构造实例化Scene对象
//    RTCScene rtcScene() const {
//        
//    }
//    
//    virtual bool fillSurfaceInteraction(const Ray &r, const Vector2f &uv, SurfaceInteraction *isect) const override;
//    
//    virtual void computeScatteringFunctions(SurfaceInteraction *isect,
//                                    MemoryArena &arena, TransportMode mode,
//                                    bool allowMultipleLobes) const override;
//    
//    const Transform & getWorldToObject() const;
//    
//    const Transform & getObjectToWorld() const;
//    
//    virtual nloJson toJson() const override {
//        return nloJson();
//    }
//    
//    static shared_ptr<GeometricPrimitive> create(const std::shared_ptr<Shape> &shape,
//                                        const std::shared_ptr<const Material> &material,
//                                        const std::shared_ptr<AreaLight> &areaLight,
//                                          const MediumInterface &mediumInterface);
//    
//private:
//
//    
//    std::shared_ptr<Shape> _shape;
//    std::shared_ptr<const Material> _material;
//    // 发光属性
//    std::shared_ptr<AreaLight> _areaLight;
//    MediumInterface _mediumInterface;
//};



class Aggregate : public Primitive {
public:

    virtual const AreaLight *getAreaLight() const override {
        DCHECK(false);
        return nullptr;
    }
    virtual const Material *getMaterial() const override {
        DCHECK(false);
        return nullptr;
    }
    virtual void computeScatteringFunctions(SurfaceInteraction *isect,
                                    MemoryArena &arena, TransportMode mode,
                                    bool allowMultipleLobes) const override {
        DCHECK(false);
    }
};

//shared_ptr<Aggregate> createAccelerator(const nloJson &data, const vector<shared_ptr<Primitive>> &);


shared_ptr<Aggregate> createAccelerator(const nloJson &data, const vector<shared_ptr<const Shape>> &);

PALADIN_END

#endif /* primitive_hpp */
