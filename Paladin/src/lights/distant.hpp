//
//  distant.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/18.
//

#ifndef distant_hpp
#define distant_hpp

#include "core/light.hpp"
#include "core/scene.hpp"

PALADIN_BEGIN

class DistantLight : public Light {
public:

    DistantLight(const Transform * LightToWorld, const Spectrum &L,
                 const Vector3f &w);
    
    virtual void preprocess(const Scene &scene) override {
        scene.worldBound().boundingSphere(&_worldCenter, &_worldRadius);
    }
    
    virtual nloJson toJson() const override {
        return nloJson();
    }
    
    virtual Spectrum sample_Le(const Point2f &u1, const Point2f &u2,
                                Float time, Ray *ray, Normal3f *nLight,
                                Float *pdfPos, Float *pdfDir) const override;
    
    virtual void pdf_Le(const Ray &ray, const Normal3f &nLight,
                        Float *pdfPos, Float *pdfDir) const override;
    
    virtual Spectrum sample_Li(const Interaction &ref, const Point2f &u, Vector3f *wi,
                       Float *pdf, VisibilityTester *vis) const override;
    
    virtual Spectrum sample_Li(DirectSamplingRecord *rcd, const Point2f &u,
                               const Scene &scene) const override;
    
    virtual Spectrum power() const override;
    
    virtual Float pdf_Li(const Interaction &, const Vector3f &) const override;
    
    virtual Float pdf_Li(const DirectSamplingRecord &) const override;
    
private:
    const Spectrum _L;
    // 光源方向，指的是光源在物体的_wLight方向
    const Vector3f _wLight;
    Point3f _worldCenter;
    Float _worldRadius;
};

CObject_ptr createDistantLight(const nloJson &param, const Arguments &lst);

PALADIN_END

#endif /* distant_hpp */
