//
//  func.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/2/7.
//

#include "func.hpp"
#include "core/scene.hpp"
#include "math/sampling.hpp"

PALADIN_BEGIN

Float infiniteLightDensity(
    const Scene &scene, const Distribution1D &lightDistr,
    const std::unordered_map<const Light *, size_t> &lightToDistrIndex,
    const Vector3f &w) {
    Float pdf = 0;
    for (const auto &light : scene.infiniteLights) {
        CHECK(lightToDistrIndex.find(light.get()) != lightToDistrIndex.end());
        size_t index = lightToDistrIndex.find(light.get())->second;
        pdf += light->pdf_Li(Interaction(), -w) * lightDistr.funcAt(index);
    }
    return pdf / (lightDistr.getFuncInt() * lightDistr.count());
}

Float correctShadingNormal(const SurfaceInteraction &isect, const Vector3f &wo,
                           const Vector3f &wi, TransportMode mode) {
    if (mode == TransportMode::Importance) {
        Float num = absDot(wo, isect.shading.normal) * absDot(wi, isect.normal);
        Float denom = absDot(wo, isect.normal) * absDot(wi, isect.shading.normal);
        if (denom == 0) {
            return 0;
        }
        return num / denom;
    }
    return 1;
}

/**
 * 原函数为
 *                    |cosθ| |cosθ'|
 *      G(p'↔p) =  -------------------- V(p'↔p)
 *                      |p - p'|^2
 *
 * 如果顶点在medium中，则不需要乘以cos
 * 
 * @param  scene   [description]
 * @param  sampler [description]
 * @param  v0      [description]
 * @param  v1      [description]
 * @return         [description]
 */
Spectrum G(const Scene &scene, Sampler &sampler, const Vertex &v0,
           const Vertex &v1) {
    Vector3f d = v0.pos() - v1.pos();
    Float g = 1 / d.lengthSquared();
    d *= std::sqrt(g);
    if (v0.isOnSurface()) {
        g *= absDot(v0.ns(), d);
    }
    if (v1.isOnSurface()) {
        g *= absDot(v1.ns(), d);
    }
    VisibilityTester vis(v0.getInteraction(), v1.getInteraction());
    return g * vis.Tr(scene, sampler);
}

PALADIN_END
