//
//  bssrdf.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/21.
//

#ifndef bssrdf_hpp
#define bssrdf_hpp

#include "interaction.hpp"
#include "bxdf.hpp"

PALADIN_BEGIN

/**
 * 之前介绍了BRDF
 * 现在介绍一下BSSRDF
 * 两者有何区别？
 * 我们知道，漫反射是由于光子进入物体表面之后，随机散射到各个方向离开物体表面
 * 入射点跟出射点距离很近，可以简化模型，当做一个点处理，我们可以用BRDF来描述
 * 物体表面反射的分布
 * 但如果由于材质原因（半透明，例如，人的皮肤，蜡烛），入射点与与出射点的距离较远
 * 不能当做一个点来处理，则需要BSSRDF来表示了，废话不多说，先看表达式
 * 
 *     Lo(po,ωo) = ∫[A] ∫[H] S(po, ωo, pi, ωi) Li(pi, ωi) |cosθi| dωidA    1式
 * 
 * BSSRDF的表现形式为 S(po, ωo, pi, ωi)
 * BSSRDF是对边界上（通常是物体表面）
 * 给定的一对点和方向之间的这些散射过程的结果进行建模的总结表示。
 *  
 */
class BSSRDF
{
public:
	BSSRDF(const SurfaceInteraction &po, Float eta) 
	: _po(po), 
	_eta(eta) {

	}

	virtual ~BSSRDF() {

	}

	virtual Spectrum S(const SurfaceInteraction &pi, const Vector3f &wi) = 0;

	virtual Spectrum sampleS(const Scene &scene, Float u1, const Point2f &u2,
                              MemoryArena &arena, SurfaceInteraction *si,
                              Float *pdf) const = 0;

protected:
	// 光线出射的位置，包含了出射方向 ωo
	const SurfaceInteraction &_po;
	// 折射率
	Float _eta;
};

PALADIN_END

#endif /* bssrdf_hpp */
