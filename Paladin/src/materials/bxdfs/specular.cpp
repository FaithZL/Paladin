//
//  specular.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/2/12.
//

#include "specular.hpp"

PALADIN_BEGIN

//  SpecularReflection
Spectrum SpecularReflection::sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample,
                  Float *pdf, BxDFType *sampledType) const {
    *wi = Vector3f(-wo.x, -wo.y, wo.z);
    *pdf = 1;
    return _fresnel->evaluate(cosTheta(*wi)) * _R / Frame::absCosTheta(*wi);
}

std::string SpecularReflection::toString() const {
    return std::string("[ SpecularReflection R: ") + _R.ToString() +
    std::string(" fresnel: ") + _fresnel->toString() + std::string(" ]");
}

// SpecularTransmission
Spectrum SpecularTransmission::sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample,
                  Float *pdf, BxDFType *sampledType) const {
    // 首先确定光线是进入或离开折射介质
    // 对象的法线都是向外的
    // 如果wo.z > 0，则说明，ray trace工作流的光线从物体外部射入物体
    bool entering = cosTheta(wo) > 0;
    Float etaI = entering ? _etaA : _etaB;
    Float etaT = entering ? _etaB : _etaA;
    // todo，这里代码可以优化一下
    if (!refract(wo, faceforward(Normal3f(0, 0, 1), wo), etaI / etaT, wi)) {
        return 0;
    }
    
    *pdf = 1;
    Spectrum ft = _T * (Spectrum(1.) - _fresnel.evaluate(cosTheta(*wi)));
    
    // 用于处理双向方法的情况，只有从光源射出的光线才需要乘以这个缩放因子
    if (_mode == TransportMode::Radiance) {
        ft *= (etaI * etaI) / (etaT * etaT);
    }
    return ft / Frame::absCosTheta(*wi);
}

std::string SpecularTransmission::toString() const {
    return std::string("[ SpecularTransmission: T: ") + _T.ToString() +
    StringPrintf(" etaA: %f etaB: %f ", _etaA, _etaB) +
    std::string(" fresnel: ") + _fresnel.toString() +
    std::string(" mode : ") +
    (_mode == TransportMode::Radiance ? std::string("RADIANCE")
     : std::string("IMPORTANCE")) +
    std::string(" ]");
}

PALADIN_END
