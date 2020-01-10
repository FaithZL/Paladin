//
//  disney.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/1/5.
//

#ifndef disney_hpp
#define disney_hpp

#include "core/material.hpp"

PALADIN_BEGIN

class DisneyMaterial : public Material {
    
public:
	DisneyMaterial(const std::shared_ptr<Texture<Spectrum>> &color,
					const std::shared_ptr<Texture<Float>> &metallic,
					const std::shared_ptr<Texture<Float>> &eta,
					const std::shared_ptr<Texture<Float>> &roughness,
					const std::shared_ptr<Texture<Float>> &specularTint,
					const std::shared_ptr<Texture<Float>> &anisotropic,
					const std::shared_ptr<Texture<Float>> &sheen,
					const std::shared_ptr<Texture<Float>> &sheenTint,
					const std::shared_ptr<Texture<Float>> &clearcoat,
					const std::shared_ptr<Texture<Float>> &clearcoatGloss,
					const std::shared_ptr<Texture<Float>> &specTrans,
					const std::shared_ptr<Texture<Spectrum>> &scatterDistance,
					bool thin,
					const std::shared_ptr<Texture<Float>> &flatness,
					const std::shared_ptr<Texture<Float>> &diffTrans,
					const std::shared_ptr<Texture<Float>> &bumpMap)
	: _color(color),
	_metallic(metallic),
	_eta(eta),
	_roughness(roughness),
	_specularTint(specularTint),
	_anisotropic(anisotropic),
	_sheen(sheen),
	_sheenTint(sheenTint),
	_clearcoat(clearcoat),
	_clearcoatGloss(clearcoatGloss),
	_specTrans(specTrans),
	_scatterDistance(scatterDistance),
	_thin(thin),
	_flatness(flatness),
	_diffTrans(diffTrans),
	_bumpMap(bumpMap) {
		
	}
    
    virtual nloJson toJson() const override {
        return nloJson();
    }
    
    virtual void computeScatteringFunctions(SurfaceInteraction *si,
                                            MemoryArena &arena,
                                            TransportMode mode,
                                            bool allowMultipleLobes) const override;
    
private:
	std::shared_ptr<Texture<Spectrum>> _color;
	std::shared_ptr<Texture<Float>> _metallic, _eta;
	std::shared_ptr<Texture<Float>> _roughness, _specularTint, _anisotropic, _sheen;
	std::shared_ptr<Texture<Float>> _sheenTint, _clearcoat, _clearcoatGloss;
	std::shared_ptr<Texture<Float>> _specTrans;
	std::shared_ptr<Texture<Spectrum>> _scatterDistance;
	bool _thin;
	std::shared_ptr<Texture<Float>> _flatness, _diffTrans, _bumpMap;
};

PALADIN_END

#endif /* disney_hpp */
