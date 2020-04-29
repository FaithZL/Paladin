//
//  microfacet.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/4/28.
//

#include "microfacet.hpp"
#include "core/bxdf.hpp"

PALADIN_BEGIN

//MicrofacetDistribution


Float Microfacet::D(const Vector3f &wh) const {

    switch (_type) {
        case Beckmann: {
            Float _tan2Theta = Frame::tanTheta2(wh);
            if (std::isinf(_tan2Theta)) {
                // 当θ为90°时，会出现tan值无穷大的情况，为了避免这种异常发生
                // 我们返回0
                return 0.;
            }
            Float cos4Theta = Frame::cosTheta2(wh) * Frame::cosTheta2(wh);
            Float ret = std::exp(-_tan2Theta * (Frame::cosPhi2(wh) / (_alphaU * _alphaV) +
                                                Frame::sinPhi2(wh) / (_alphaV * _alphaV))) /
            (Pi * _alphaU * _alphaU * cos4Theta);
            return ret;
        }
        case GGX: {
            Float _cosTheta = Frame::absCosTheta(wh);
            if (_cosTheta == 1) {
                // 当θ为90°时，会出现tan值无穷大的情况，为了避免这种异常发生
                // 我们返回0
                return 0.;
            }
            Float cosTheta4 = (_cosTheta * _cosTheta) * (_cosTheta * _cosTheta);
            Float _tanTheta2 = Frame::tanTheta2(wh);

            Float sinPhi2 = Frame::sinPhi2(wh);
            Float cosPhi2 = Frame::cosPhi2(wh);

            Float alphax2 = _alphaU * _alphaU;
            Float alphay2 = _alphaV * _alphaV;

            Float term1 = 1 + (cosPhi2 / alphax2 + sinPhi2 / alphay2) * _tanTheta2;
            Float term2 = term1 * term1;
            Float ret = 1.0 / (Pi * _alphaU * _alphaV * cosTheta4 * term2);
            return ret;
        }
        default: {
            return 0;
        }
    }
}

Float Microfacet::lambda(const Vector3f &wh) const {
    switch (_type) {
        case Beckmann: {
            Float absTanTheta = std::abs(Frame::tanTheta(wh));
            if (std::isinf(absTanTheta)) {
                // 当θ为90°时，会出现tan值无穷大的情况，为了避免这种异常发生
                // 我们返回0
                return 0.;
            }
            // α^2 = (cosθh)^2/αx^2 + (sinθh)^2/αy^2)
            Float alpha = std::sqrt(Frame::cosPhi2(wh) * _alphaU * _alphaU + Frame::sinPhi2(wh) * _alphaV * _alphaV);
            Float a = 1 / (alpha * absTanTheta);
            if (a >= 1.6f) {
                return 0;
            }
            return (1 - 1.259f * a + 0.396f * a * a) / (3.535f * a + 2.181f * a * a);
        }
        
        case GGX: {
            Float absTanTheta = std::abs(Frame::tanTheta(wh));
            if (std::isinf(absTanTheta)) {
                // 当θ为90°时，会出现tan值无穷大的情况，为了避免这种异常发生
                // 我们返回0
                return 0.;
            }
            // α^2 = (cosθh)^2/αx^2 + (sinθh)^2/αy^2)
            Float alpha = std::sqrt(Frame::cosPhi2(wh) * _alphaU * _alphaU + Frame::sinPhi2(wh) * _alphaV * _alphaV);
            Float alpha2Tan2Theta = (alpha * absTanTheta) * (alpha * absTanTheta);
            return (-1 + std::sqrt(1.f + alpha2Tan2Theta)) / 2;
        }
            
        default:
            break;
    }
}

Vector3f Microfacet::sample_wh(const Vector3f &wo, const Point2f &u) const {
    switch (_type) {
        case Beckmann: {
            // 忽略遮挡
            Float tan2Theta, phi;
            if (_alphaU == _alphaV) {
                // 计算各项同性
                Float logSample = std::log(1 - u[0]);
                DCHECK(!std::isinf(logSample));
                tan2Theta = -_alphaU * _alphaU * logSample;
                phi = u[1] * 2 * Pi;
            } else {
                // 计算各向异性
                Float logSample = std::log(1 - u[0]);
                DCHECK(!std::isinf(logSample));
                phi = std::atan(_alphaV / _alphaU *
                                std::tan(2 * Pi * u[1] + 0.5f * Pi));
                if (u[1] > 0.5f) phi += Pi;
                Float sinPhi = std::sin(phi), cosPhi = std::cos(phi);
                Float alphax2 = _alphaU * _alphaU, alphay2 = _alphaV * _alphaV;
                tan2Theta = -logSample /
                        (cosPhi * cosPhi / alphax2 + sinPhi * sinPhi / alphay2);
            }
            
            Float cosTheta = 1 / std::sqrt(1 + tan2Theta);
            Float sinTheta = std::sqrt(std::max((Float)0, 1 - cosTheta * cosTheta));
            Vector3f wh = sphericalDirection(sinTheta, cosTheta, phi);
            if (!sameHemisphere(wo, wh)) {
                wh = -wh;
            }
            return wh;
        }
            
        case GGX: {
            Float cosTheta = 0, phi = (2 * Pi) * u[1];
            if (_alphaU == _alphaV) {
                // 计算各向同性
                Float tanTheta2 = _alphaU * _alphaU * u[0] / (1.0f - u[0]);
                cosTheta = 1 / std::sqrt(1 + tanTheta2);
            } else {
                // 计算各向异性
                phi = std::atan(_alphaV / _alphaU * std::tan(2 * Pi * u[1] + .5f * Pi));
                if (u[1] > .5f) {
                    phi += Pi;
                }
                Float sinPhi = std::sin(phi), cosPhi = std::cos(phi);
                const Float alphax2 = _alphaU * _alphaU, alphay2 = _alphaV * _alphaV;
                const Float alpha2 = 1 / (cosPhi * cosPhi / alphax2 + sinPhi * sinPhi / alphay2);
                Float tanTheta2 = alpha2 * u[0] / (1 - u[0]);
                cosTheta = 1 / std::sqrt(1 + tanTheta2);
            }
            Float sinTheta = std::sqrt(std::max((Float)0., (Float)1. - cosTheta * cosTheta));
            Vector3f wh = sphericalDirection(sinTheta, cosTheta, phi);
            if (!sameHemisphere(wo, wh)) {
                wh = -wh;
            }
            return wh;
        }
        
            
        default:
            break;
    }
}

Float Microfacet::pdfDir(const Vector3f &wo, const Vector3f &wh) const {
    if (_sampleVisibleArea) {
        // 如果只计算wo视角可见部分，则需要乘以史密斯遮挡函数再归一化
        // 归一化方式如下，利用以上3式
        // cosθo = ∫[hemisphere]G1(ωo,ωh) max(0, ωo · ωh) D(ωh)dωh
        // 将cos项移到右边，得
        // 1 = ∫[hemisphere]G1(ωo,ωh) max(0, ωo · ωh) D(ωh) / (cosθo) dωh
        // 则概率密度函数为 G1(ωo,ωh) max(0, ωo · ωh) D(ωh) / (cosθo) ,代码如下
        return D(wh) * G1(wo) * absDot(wo, wh) / Frame::absCosTheta(wo);
    } else {
        // 如果忽略几何遮挡，则概率密度函数值就是D(ωh) * cosθh
        return D(wh) * Frame::absCosTheta(wh);
    }
}

PALADIN_END
