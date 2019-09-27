//
//  material.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "material.hpp"
#include "primitive.hpp"
#include "texture.hpp"
#include "spectrum.hpp"
#include "bxdf.hpp"


PALADIN_BEGIN

// p'(u,v) = p(u,v) + d(u,v)n(u,v)
// 用人话来说就是p点经过bump贴图映射后的坐标为原始坐标加上bump偏移乘以法向量
// 这点还是非常容易理解的
// 但我们还忽略了一个点就是两个纹理坐标之间p是如何过渡的，其实就是该点uv方向的切线
void Material::bump(const std::shared_ptr<Texture<Float>> &d, SurfaceInteraction *si) {
	SurfaceInteraction siEval = *si;

	Float du = 0.5f * (std::abs(si->dudx) + std::abs(si->dudy));
}


PALADIN_END