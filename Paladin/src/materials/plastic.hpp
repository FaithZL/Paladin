//
//  plastic.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/9/24.
//

#ifndef plastic_hpp
#define plastic_hpp

#include "core/material.hpp"

PALADIN_BEGIN

/**
 * 塑料材质
 * 由高光反射与漫反射构成
 * 漫反射部分为Lambertian
 * 高光反射为MicrofacetReflection
 */
class PlasticMaterial : public Material {
public:
    PlasticMaterial(const std::shared_ptr<Texture<Spectrum>> &Kd,
                    const std::shared_ptr<Texture<Spectrum>> &Ks,
                    const std::shared_ptr<Texture<Float>> &roughness,
                    const std::shared_ptr<Texture<Float>> &bumpMap,
                    bool remapRoughness)
	:_Kd(Kd),
	_Ks(Ks),
	_roughness(roughness),
	_bumpMap(bumpMap),
	_remapRoughness(remapRoughness) {

	}

    void computeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena,
                                    TransportMode mode,
                                    bool allowMultipleLobes) const;

private:
    std::shared_ptr<Texture<Spectrum>> _Kd, _Ks;
    std::shared_ptr<Texture<Float>> _roughness, _bumpMap;
    const bool _remapRoughness;
};

PALADIN_END

#endif /* plastic_hpp */
