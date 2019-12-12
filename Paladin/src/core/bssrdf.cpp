//
//  bssrdf.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/21.
//

#include "bssrdf.hpp"

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

    // Compute reduced scattering coefficients $\sigmaps, \sigmapt$ and albedo
    // $\rhop$
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
