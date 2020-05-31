//
//  material.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "material.hpp"
#include "aggregate.hpp"
#include "texture.hpp"
#include "spectrum.hpp"
#include "bxdf.hpp"
#include "tools/parallel.hpp"
#include "materials/bxdfs/bsdf.hpp"

PALADIN_BEGIN

Material::Material(const shared_ptr<Texture<Spectrum>> &normalMap,
         const shared_ptr<Texture<Float>> &bumpMap,
         Float scale)
: _normalMap(normalMap),
_bumpMap(bumpMap),
_normalMapScale(scale) {
    
}

void Material::initScatteringFunctions() {
    int nThread = maxThreadIndex();
    _bsdfs.reserve(nThread);
    for (int i = 0; i < nThread; ++i) {
        auto bsdf = make_shared<BSDF>(1.f);
        _bsdfs.push_back(bsdf);
        initBSDF(bsdf.get());
    }
}

void Material::bumpMapping(const std::shared_ptr<Texture<Float>> &d, SurfaceInteraction *si) {
	SurfaceInteraction siEval = *si;

	//todo 这里不是很理解为何要将两个方向相加
	// 取像素间隔的一半，所以乘以0.5
	Float du = 0.5f * (std::abs(si->dudx) + std::abs(si->dudy));
	if (du == 0) {
		du = 0.0005f;
	}
	// 计算u微小偏移之后pos的值
	siEval.pos = si->pos + du * si->shading.dpdu;
	// 新的uv坐标
	siEval.uv = si->uv + Vector2f(du, 0);
	// 由于法线可能被翻转，所以需要重新计算
	Normal3f tmpNormal = (Normal3f)cross(si->shading.dpdu, si->shading.dpdv);
	// 随u变化之后的法线方向
	siEval.normal = normalize(tmpNormal + du * si->dndu);
	// 计算经过u方向微小偏移之后的bump值
	Float uDisplace = d->evaluate(siEval);

	// 计算方式同u
	Float dv = 0.5f * (std::abs(si->dvdx) + std::abs(si->dvdy));
	if (dv == 0) {
		dv = 0.0005f;
	}
	siEval.pos = si->pos + dv * si->shading.dpdv;
	siEval.uv = si->uv + Vector2f(0.f, dv);
	siEval.normal = normalize(tmpNormal + dv * si->dndv);
	Float vDisplace = d->evaluate(siEval);

	// 偏移之前的bump值
	Float diplace = d->evaluate(*si);
	/**
	 * 计算微分几何信息，其实就是用这个表达式
	 *  dp'(u,v)	dp(u,v)	   db(u+△u,v) - db(u,v)                  dn(u,v)
	 * --------- ≈ --------- + --------------------- n(u,v) + b(u,v) ------
	 *     du		  du              △u                               du
	 */
    Vector3f dpdu = si->shading.dpdu
                + (uDisplace - diplace) / du * Vector3f(si->shading.normal)
                + diplace * Vector3f(si->shading.dndu);

    Vector3f dpdv = si->shading.dpdv
                + (vDisplace - diplace) / dv * Vector3f(si->shading.normal)
                + diplace * Vector3f(si->shading.dndv);

	si->setShadingGeometry(dpdu,
                           dpdv,
                           si->shading.dndu,
                           si->shading.dndv,
                           false);
    
}

void Material::normalMapping(const shared_ptr<Texture<Spectrum> > &normalMap,
                             SurfaceInteraction *si, Float scale/* = -1*/) {
    si->computeTangentSpace();
    if (!si->tangentSpace.isValid()) {
        return;
    }
    
    Spectrum color = normalMap->evaluate(*si);
    Float rgb[3];
    color.ToRGB(rgb);
    Vector3f normal(color[0], color[1], color[2]);
    normal = normal * 2.0f - Vector3f(1, 1, 1);
    
    normal.x *= scale;
    normal.y *= scale;
    
    Vector2f v2 = Vector2f(normal.x, normal.y);
    normal.z = std::sqrt((Float)(1.f - clamp(v2.lengthSquared(), 0.f, 1.f)));
    
    Vector3f worldNormal = si->tangentSpace.toWorld(normal);
    
    si->shading.normal = normalize(Normal3f(worldNormal));
    coordinateSystem((Vector3f)si->shading.normal, &si->shading.dpdu, &si->shading.dpdv);
    si->normal = faceforward(si->normal, si->shading.normal);
}

//"matte" : {
//    "type" : "matte",
//    "param" : {
//        "Kd" : "tex1"
//    }
//}
Material * createMaterial(const nloJson &data) {
    string type = data.value("type", "matte");
    nloJson param = data["param"];
    auto creator = GET_CREATOR(type);
    auto ret = dynamic_cast<Material *>(creator(param, {}));
    return ret;
}

PALADIN_END
