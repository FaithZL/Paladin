//
//  bssrdf.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/21.
//

#include "bssrdf.hpp"
#include "math/interpolation.hpp"
#include "tools/parallel.hpp"

PALADIN_BEGIN

// 用多项式去拟合积分曲线
Float FresnelMoment1(Float eta) {
    Float eta2 = eta * eta, eta3 = eta2 * eta, eta4 = eta3 * eta,
          eta5 = eta4 * eta;
    if (eta < 1)
        return 0.45966f - 1.73965f * eta + 3.37668f * eta2 - 3.904945 * eta3 +
               2.49277f * eta4 - 0.68441f * eta5;
    else
        return -4.61686f + 11.1136f * eta - 10.4646f * eta2 + 5.11455f * eta3 -
               1.27198f * eta4 + 0.12746f * eta5;
}

// 用多项式去拟合积分曲线
Float FresnelMoment2(Float eta) {
    Float eta2 = eta * eta, eta3 = eta2 * eta, eta4 = eta3 * eta,
          eta5 = eta4 * eta;
    if (eta < 1) {
        return 0.27614f - 0.87350f * eta + 1.12077f * eta2 - 0.65095f * eta3 +
               0.07883f * eta4 + 0.04860f * eta5;
    } else {
        Float r_eta = 1 / eta, r_eta2 = r_eta * r_eta, r_eta3 = r_eta2 * r_eta;
        return -547.033f + 45.3087f * r_eta3 - 218.725f * r_eta2 +
               458.843f * r_eta + 404.557f * eta - 189.519f * eta2 +
               54.9327f * eta3 - 9.00603f * eta4 + 0.63942f * eta5;
    }
}

Float beamDiffusionMS(Float sigma_s, Float sigma_a, Float g, Float eta,
                      Float r) {
    const int nSamples = 100;
    Float Ed = 0;
    // Precompute information for dipole integrand

    // 根据相似理论计算对应的修正系数
    // σ's = (1 - g)σs
    // σ't = σa + σ's
    // ρ' = σ's / σ't
    Float sigmap_s = sigma_s * (1 - g);
    Float sigmap_t = sigma_a + sigmap_s;
    Float rhop = sigmap_s / sigmap_t;

    // Compute non-classical diffusion coefficient $D_\roman{G}$ using
    // Equation (15.24)
    Float D_g = (2 * sigma_a + sigmap_s) / (3 * sigmap_t * sigmap_t);

    // Compute effective transport coefficient $\sigmatr$ based on $D_\roman{G}$
    Float sigma_tr = std::sqrt(sigma_a / D_g);

    // Determine linear extrapolation distance $\depthextrapolation$ using
    // Equation (15.28)
    Float fm1 = FresnelMoment1(eta), fm2 = FresnelMoment2(eta);
    Float ze = -2 * D_g * (1 + 3 * fm2) / (1 - 2 * fm1);

    // Determine exitance scale factors using Equations (15.31) and (15.32)
    Float cPhi = .25f * (1 - 2 * fm1), cE = .5f * (1 - 3 * fm2);
    for (int i = 0; i < nSamples; ++i) {
        // Sample real point source depth $\depthreal$
        Float zr = -std::log(1 - (i + .5f) / nSamples) / sigmap_t;

        // Evaluate dipole integrand $E_{\roman{d}}$ at $\depthreal$ and add to
        // _Ed_
        Float zv = -zr + 2 * ze;
        Float dr = std::sqrt(r * r + zr * zr), dv = std::sqrt(r * r + zv * zv);

        // Compute dipole fluence rate $\dipole(r)$ using Equation (15.27)
        Float phiD = Inv4Pi / D_g * (std::exp(-sigma_tr * dr) / dr -
                                     std::exp(-sigma_tr * dv) / dv);

        // Compute dipole vector irradiance $-\N{}\cdot\dipoleE(r)$ using
        // Equation (15.27)
        Float EDn = Inv4Pi * (zr * (1 + sigma_tr * dr) *
                                  std::exp(-sigma_tr * dr) / (dr * dr * dr) -
                              zv * (1 + sigma_tr * dv) *
                                  std::exp(-sigma_tr * dv) / (dv * dv * dv));

        // Add contribution from dipole for depth $\depthreal$ to _Ed_
        Float E = phiD * cPhi + EDn * cE;
        Float kappa = 1 - std::exp(-2 * sigmap_t * (dr + zr));
        Ed += kappa * rhop * rhop * E;
    }
    return Ed / nSamples;
}

Float beamDiffusionSS(Float sigma_s, Float sigma_a, Float g, Float eta,
                      Float r) {
    // Compute material parameters and minimum $t$ below the critical angle
    Float sigma_t = sigma_a + sigma_s, rho = sigma_s / sigma_t;
    Float tCrit = r * std::sqrt(eta * eta - 1);
    Float Ess = 0;
    const int nSamples = 100;
    for (int i = 0; i < nSamples; ++i) {
        // Evaluate single scattering integrand and add to _Ess_
        Float ti = tCrit - std::log(1 - (i + .5f) / nSamples) / sigma_t;

        // Determine length $d$ of connecting segment and $\cos\theta_\roman{o}$
        Float d = std::sqrt(r * r + ti * ti);
        Float cosThetaO = ti / d;

        // Add contribution of single scattering at depth $t$
        Ess += rho * std::exp(-sigma_t * (d + tCrit)) / (d * d) *
        phaseHG(cosThetaO, g) * (1 - FrDielectric(-cosThetaO, 1, eta)) *
               std::abs(cosThetaO);
    }
    return Ess / nSamples;
}

// 生成BSSRDFTable中的数据
void computeBeamDiffusionBSSRDF(Float g, Float eta, BSSRDFTable *t) {
    // 光学半径的样本列表 0   0.0025
    t->radiusSamples[0] = 0;
    t->radiusSamples[1] = 2.5e-3f;
    // 生成光学半径样本列表，一般来说，64个样本
    for (int i = 2; i < t->nRadiusSamples; ++i) {
        t->radiusSamples[i] = t->radiusSamples[i - 1] * 1.2f;
    }

    // 生成反射率样本，一般来说，100个样本
    // 样本点的通项公式如下
    // 
    //        1 - e^(-8i/(N-1))
    // ρi = --------------------
    //           1 - e^(-8)
    for (int i = 0; i < t->nRhoSamples; ++i) {
        t->rhoSamples[i] =
            (1 - std::exp(-8 * i / (Float)(t->nRhoSamples - 1))) /
            (1 - std::exp(-8));
    }

    // 并行的生成Sr函数表
    parallelFor([&](int i) {
        // 计算第i个反射率样本所对应的的nRadiusSamples个半径样本的函数值
        // 难点就在于下面两个函数的实现
        for (int j = 0; j < t->nRadiusSamples; ++j) {
            Float rho = t->rhoSamples[i], r = t->radiusSamples[j];
            t->profile[i * t->nRadiusSamples + j] =
                2 * Pi * r * (beamDiffusionSS(rho, 1 - rho, g, eta, r) +
                              beamDiffusionMS(rho, 1 - rho, g, eta, r));
        }

        // 计算有效反射率与反射率之间的关系，还有Sr的cdf用于重要性采样
        t->rhoEff[i] =
            IntegrateCatmullRom(t->nRadiusSamples, t->radiusSamples.get(),
                                &t->profile[i * t->nRadiusSamples],
                                &t->profileCDF[i * t->nRadiusSamples]);
    }, t->nRhoSamples);
}


BSSRDFTable::BSSRDFTable(int nRhoSamples, int nRadiusSamples)
: nRhoSamples(nRhoSamples),
nRadiusSamples(nRadiusSamples),
rhoSamples(new Float[nRhoSamples]),
radiusSamples(new Float[nRadiusSamples]),
profile(new Float[nRadiusSamples * nRhoSamples]),
rhoEff(new Float[nRhoSamples]),
profileCDF(new Float[nRadiusSamples * nRhoSamples]) {

}

Spectrum TabulatedBSSRDF::Sr(Float r) const {
    Spectrum ret(0.f);
    
    for (int ch = 0; ch < Spectrum::nSamples; ++ch) {
        Float rOptical = r * _sigma_t[ch];
        int rhoOffset, radiusOffset;
        Float rhoWeights[4], radiusWeights[4];
    }
    return ret;
}

Float TabulatedBSSRDF::pdf_Sr(int ch, Float distance) const {
    return 0;
}

Float TabulatedBSSRDF::sample_Sr(int ch, Float sample) const {
    return 0;
}

PALADIN_END
