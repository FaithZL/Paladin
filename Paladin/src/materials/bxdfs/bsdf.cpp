//
//  bsdf.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/2/13.
//

#include "bsdf.hpp"
#include "core/interaction.hpp"

PALADIN_BEGIN

// BSDF
BSDF::BSDF(const SurfaceInteraction &si, Float eta/* = 1*/)
: eta(eta),
_gNormal(si.normal),
_sNormal(si.shading.normal),
_sTangent(normalize(si.shading.dpdu)),
_tTangent(cross(_sNormal, _sTangent)) {

}

BSDF::BSDF(Float eta)
: eta(eta) {
    
}

void BSDF::addBxDF(const shared_ptr<BxDF> &b) {
    _bxdfs.push_back(b);
}

void BSDF::updateGeometry(const SurfaceInteraction &si) {
    _gNormal = si.normal;
    _sNormal = si.shading.normal;
    _sTangent = normalize(si.shading.dpdu);
    _tTangent = cross(_sNormal, _sTangent);
}

Spectrum BSDF::f(const Vector3f &woW, const Vector3f &wiW, BxDFType flags) const {
    TRY_PROFILE(Prof::BSDFEvaluation)
    Vector3f wi = worldToLocal(wiW);
    Vector3f wo = worldToLocal(woW);
    if (wo.z == 0) {
        return 0.;
    }
    bool reflect = dot(wiW, _gNormal) * dot(woW, _gNormal) > 0;
    Spectrum f(0.f);
    for (int i = 0; i < nBxDFs; ++i) {
        if (bxdfs[i]->matchesFlags(flags) &&
            ((reflect && (bxdfs[i]->type & BSDF_REFLECTION)) ||
             (!reflect && (bxdfs[i]->type & BSDF_TRANSMISSION)))) {
            f += bxdfs[i]->f(wo, wi);
        }
    }
    return f;
}

Spectrum BSDF::rho_hh(int nSamples, const Point2f *samples1,
                   const Point2f *samples2, BxDFType flags) const {
    Spectrum ret(0.f);
    for (int i = 0; i < nBxDFs; ++i) {
        if (bxdfs[i]->matchesFlags(flags)) {
            ret += bxdfs[i]->rho_hh(nSamples, samples1, samples2);
        }
    }
    return ret;
}

Spectrum BSDF::rho_hd(const Vector3f &wo, int nSamples, const Point2f *samples,
                   BxDFType flags) const {
    Spectrum ret(0.f);
    for (int i = 0; i < nBxDFs; ++i) {
        if (bxdfs[i]->matchesFlags(flags)) {
            ret += bxdfs[i]->rho_hd(wo, nSamples, samples);
        }
    }
    return ret;
}

Spectrum BSDF::sample_f(const Vector3f &woWorld, Vector3f *wiWorld,
                        const Point2f &u, Float *pdf, BxDFType type,
                        BxDFType *sampledType) const {
    TRY_PROFILE(Prof::BSDFSampling)
    // 计算符合type反射类型的组件数量
    int matchingComps = numComponents(type);
    if (matchingComps == 0) {
        *pdf = 0;
        if (sampledType) {
            *sampledType = BxDFType(0);
        }
        return Spectrum(0);
    }
    // 根据随机变量随机选择bxdf组件
    // 目前的BSDF的各个bxdf组件是均匀分布，todo改为加权形式
    int comp = std::min((int)std::floor(u[0] * matchingComps), matchingComps - 1);
    
    BxDF *bxdf = nullptr;
    int count = comp;
    // 找到第comp个目标组件
    for (int i = 0; i < nBxDFs; ++i) {
        if (bxdfs[i]->matchesFlags(type) && count-- == 0) {
            bxdf = bxdfs[i];
            break;
        }
    }
    DCHECK(bxdf != nullptr);
    VLOG(2) << "BSDF::Sample_f chose comp = " << comp << " / matching = " <<
    matchingComps << ", bxdf: " << bxdf->toString();
    // 重新映射二维随机变量
    Point2f uRemapped(std::min(u[0] * matchingComps - comp, OneMinusEpsilon), u[1]);
    
    Vector3f wi, wo = worldToLocal(woWorld);
    if (wo.z == 0) {
        return 0.;
    }
    *pdf = 0;
    if (sampledType) {
        *sampledType = bxdf->type;
    }

    // 对选中的bxdf采样
    Spectrum f = bxdf->sample_f(wo, &wi, uRemapped, pdf, sampledType);

    VLOG(2) << "For wo = " << wo << ", sampled f = " << f << ", pdf = "
    << *pdf << ", ratio = " << ((*pdf > 0) ? (f / *pdf) : Spectrum(0.))
    << ", wi = " << wi;
    if (*pdf == 0) {
        if (sampledType) {
            *sampledType = BxDFType(0);
        }
        return 0;
    }
    *wiWorld = localToWorld(wi);
    
    // 如果不包含高光反射，并且匹配的组件数大于1，才会累加pdf
    // 因为高光反射不能使用常规方法采样
    if (!(bxdf->type & BSDF_SPECULAR) && matchingComps > 1) {
        for (int i = 0; i < nBxDFs; ++i) {
            if (bxdfs[i] != bxdf && bxdfs[i]->matchesFlags(type)) {
                Float tmp = bxdfs[i]->pdfDir(wo, wi);
                DCHECK(tmp >= 0);
                *pdf += tmp;
            }
        }
    }

    if (matchingComps > 1) {
        *pdf /= matchingComps;
    }
    
    if (!(bxdf->type & BSDF_SPECULAR)) {
        bool reflect = dot(*wiWorld, _gNormal) * dot(woWorld, _gNormal) > 0;
        f = 0.;
        for (int i = 0; i < nBxDFs; ++i) {
            if (bxdfs[i]->matchesFlags(type) &&
                ((reflect && (bxdfs[i]->type & BSDF_REFLECTION)) ||
                (!reflect && (bxdfs[i]->type & BSDF_TRANSMISSION)))) {
                f += bxdfs[i]->f(wo, wi);
            }
        }
    }
    VLOG(2) << "Overall f = " << f << ", pdf = " << *pdf << ", ratio = "
    << ((*pdf > 0) ? (f / *pdf) : Spectrum(0.));
    DCHECK(*pdf >= 0);
    return f;
}

Float BSDF::pdfDir(const Vector3f &woWorld, const Vector3f &wiWorld,
                BxDFType flags) const {
    TRY_PROFILE(Prof::BSDFPdf)
    if (nBxDFs == 0.f) {
        return 0.f;
    }
    Vector3f wo = worldToLocal(woWorld);
    Vector3f wi = worldToLocal(wiWorld);
    if (wo.z == 0) return 0.;
    Float pdf = 0.f;
    int matchingComps = 0;
    for (int i = 0; i < nBxDFs; ++i) {
        if (bxdfs[i]->matchesFlags(flags)) {
            ++matchingComps;
            pdf += bxdfs[i]->pdfDir(wo, wi);
        }
    }
    Float v = matchingComps > 0 ? pdf / matchingComps : 0.f;
    return v;
}

std::string BSDF::toString() const {
    std::string s = StringPrintf("[ BSDF eta: %f nBxDFs: %d", eta, nBxDFs);
    for (int i = 0; i < nBxDFs; ++i)
        s += StringPrintf("\n  bxdfs[%d]: ", i) + bxdfs[i]->toString();
    return s + std::string(" ]");
}




PALADIN_END
