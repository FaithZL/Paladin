//
//  matte.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef matte_hpp
#define matte_hpp

#include "core/header.h"
#include "core/material.hpp"
#include "tools/classfactory.hpp"

PALADIN_BEGIN

/**
 * 无光的粗糙的材质
 * 如果粗糙度为零，则使用lambertian反射
 * 否则使用OrenNayar反射模型
 */
class MatteMaterial : public Material {
public:
	MatteMaterial(const std::shared_ptr<Texture<Spectrum>> &Kd,
                  const std::shared_ptr<Texture<Float>> &sigma,
                  const std::shared_ptr<Texture<Float>> &bumpMap)
    : _Kd(Kd), _sigma(sigma), _bumpMap(bumpMap) {

    }
    
    virtual nloJson toJson() const override {
        return nloJson();
    }

	virtual void computeScatteringFunctions(SurfaceInteraction *si,
									MemoryArena &arena,
                                    TransportMode mode,
                                    bool allowMultipleLobes) const override;

private:
    // 漫反射系数
    std::shared_ptr<Texture<Spectrum>> _Kd;
    // 粗糙度
    std::shared_ptr<Texture<Float>> _sigma;
    // bump贴图
    std::shared_ptr<Texture<Float>> _bumpMap;    
	
};

/**
 * 为光源创建的默认材质
 * @return
 */
shared_ptr<const MatteMaterial> createLightMat();

CObject_ptr createMatte(const nloJson &, const Arguments&);

PALADIN_END

#endif /* matte_hpp */
