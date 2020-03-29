//
//  fourierbsdf.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/2/15.
//

#include "fourierbsdf.hpp"
#include "math/interpolation.hpp"

PALADIN_BEGIN


// 傅里叶BSDF
Spectrum FourierBSDF::f(const Vector3f &wo, const Vector3f &wi) const {
    // 这里为什么是-wi？
    // 可能外部读取的数据默认是wi是从外部指向入射点的向量
    // 所以要取反
    Float muI = Frame::cosTheta(-wi);
    Float muO = Frame::cosTheta(wo);
    Float cosPhi = cosDPhi(-wi, wo);
    
    int offsetI, offsetO;
    Float weightsI[4], weightsO[4];
    if (!_bsdfTable.getWeightsAndOffset(muI, &offsetI, weightsI) ||
        !_bsdfTable.getWeightsAndOffset(muO, &offsetO, weightsO)) {
        return Spectrum(0.f);
    }
    
    Float *ak = ALLOCA(Float, _bsdfTable.mMax * _bsdfTable.nChannels);
    memset(ak, 0, _bsdfTable.mMax * _bsdfTable.nChannels * sizeof(Float));
    
    int mMax = 0;
    for (int b = 0; b < 4; ++b) {
        for (int a = 0; a < 4; ++a) {
            Float weight = weightsI[a] * weightsO[b];
            if (weight != 0) {
                int m;
                const Float *ap = _bsdfTable.getAk(offsetI + a, offsetO + b, &m);
                mMax = std::max(mMax, m);
                for (int c = 0; c < _bsdfTable.nChannels; ++c) {
                    for (int k = 0; k < mMax; ++k) {
                        ak[c * _bsdfTable.mMax + k] += weight * ap[c * m + k];
                    }
                }
            }
        }
    }
    
    Float Y = std::max((Float)0, Fourier(ak, mMax, cosPhi));
    // 8.21式中的|μi|
    Float scale = muI != 0 ? (1 / std::abs(muI)) : (Float)0;

    if (_mode == TransportMode::Radiance && muI * muO > 0) {
        // 经过折射之后有缩放
        float eta = muI > 0 ? 1 / _bsdfTable.eta : _bsdfTable.eta;
        scale *= eta * eta;
    }
    
    if (_bsdfTable.nChannels == 1)
        return Spectrum(Y * scale);
    else {
        Float R = Fourier(ak + 1 * _bsdfTable.mMax, mMax, cosPhi);
        Float B = Fourier(ak + 2 * _bsdfTable.mMax, mMax, cosPhi);
        Float G = 1.39829f * Y - 0.100913f * B - 0.297375f * R;
        Float rgb[3] = {R * scale, G * scale, B * scale};
        return Spectrum::FromRGB(rgb).clamp();
    }
}

Spectrum FourierBSDF::sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                  Float *pdf, BxDFType *sampledType) const {
    return Spectrum(1.f);
}

Float FourierBSDF::pdfDir(const Vector3f &wo, const Vector3f &wi) const {
    return 0;
}

std::string FourierBSDF::toString() const {
    // todo
    return "";
}

bool FourierBSDFTable::getWeightsAndOffset(Float cosTheta, int *offset,
                                       Float weights[4]) const {
    return CatmullRomWeights(nMu, mu, cosTheta, offset, weights);
}

PALADIN_END
