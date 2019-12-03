//
//  bssrdf.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/21.
//

#ifndef bssrdf_hpp
#define bssrdf_hpp

#include "interaction.hpp"
#include "core/spectrum.hpp"
#include "core/bxdf.hpp"

PALADIN_BEGIN

/**
 * 参考资料
 * http://www.pbr-book.org/3ed-2018/Volume_Scattering/The_BSSRDF.html
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
class BSSRDF {
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

// 菲涅尔函数一阶矩
// ∫[0,π/2]Fr(η, cosθi)sinθcosθdθ
Float FresnelMoment1(Float invEta);

// 菲涅尔函数二阶矩
Float FresnelMoment2(Float invEta);

/**
 * 
 * BSSRDF的通用性比较強
 * 即使在比较简单的几何图形下求解光照传输的解决方案已经非常的困难了
 * 例如球体，平面等等，而BSSRDF可以附加到任何一个几何体上，导致求解
 * BSSRDF的积分是不现实的，接下来要介绍一种简单的表示
 * 
 * 我们可以做如下近似简化
 * 
 *     S(po, ωo, pi, ωi) ≈ (1 - Fr(cosθo)) Sp(po, pi) Sω(ωi)  2式
 * 
 * 先来分析一下Sω(ωi)函数，该函数可以认为是一个菲涅尔透射的缩放版本
 *
 *     Sω(ωi) = (1 - Fr(cosθi)) / cπ         3式
 *
 * 来看看分子部分，1 - Fr(cosθi)表示光子进入物体表面的部分
 * 分母部分为π是因为理想漫反射的brdf为1/π，c是常数，为了保证 ∫[H] Sω(ωi) cosθi dωi = 1
 * 
 * 将 3式带入 ∫[H] Sω(ω) cosθ dω = 1，可得
 *
 *     1 = ∫[H] ((1 - Fr(η, cosθ)) / cπ) cosθ dω
 *
 *     求解得到 c = 1 - 2 * ∫[0,π/2]Fr(η, cosθ)sinθcosθdθ
 *
 * 式中的积分被称为菲涅尔函数的一阶矩（个人理解这应该是近似函数）
 * 也有其他涉及余弦函数高次幂的矩，并且经常出现在与次表面散射相关的计算中，
 * 更一般的表示方法，菲涅尔的第i阶矩：
 * _
 * Fr(η,i) = ∫[0,π/2] Fr(η, cosθ) sinθ (cosθ)^i dθ
 *
 * pbrt提供了FresnelMoment1()，FresnelMoment2()
 * 两个函数用多项式的方式去拟合菲涅尔的一阶矩跟二阶矩，但有个微妙的地方是
 * 这两个函数接受了eta的倒数
 *
 * 接下来分析Sp(po, pi)项，我们假设表面不仅局部平坦，而且任意等距离的两点的Sp函数相同
 * 
 *                Sp(po, pi) ≈ Sr(|po - pi|)
 *
 * SeparableBSSRDF 就做了如上的近似简化
 * 
 */
class SeparableBSSRDF : public BSSRDF {
public:
	SeparableBSSRDF(const SurfaceInteraction &po, Float eta,
					const Material *material, TransportMode mode)
	: BSSRDF(po, eta),
	_sNormal(po.shading.normal),
	_material(material),
	_sTangent(normalize(po.shading.dpdu)),
	_tTangent(cross(_sNormal, _sTangent)),
	_mode(mode){

	}

	// S(po, ωo, pi, ωi) ≈ (1 - Fr(cosθo)) Sp(po, pi) Sω(ωi)
	Spectrum S(const SurfaceInteraction &pi, const Vector3f &wi) {
		// todo 这里的折射率应该要传入，待优化
		Float Ft = frDielectric(cosTheta(_po.wo), 1, _eta);
		return (1 - Ft) * Sp(pi) * Sw(wi);
	}

	// Sω(ωi) = (1 - Fr(cosθi)) / cπ
	// c = 1 - 2 * ∫[0,π/2]Fr(η, cosθi)sinθcosθdθ
	Spectrum Sw(const Vector3f &w) const {
		Float c = 1 - 2 * FresnelMoment1(1 / _eta);
		return (1 - frDielectric(cosTheta(w), 1, _eta)) / (c * Pi);
	}

	// Sp(po, pi) ≈ Sr(|po - pi|)
	Spectrum Sp(const SurfaceInteraction &pi) const {
		return Sr(distance(_po.pos, pi.pos));
	}

	Spectrum sample_S(const Scene &scene, Float u1, const Point2f &u2,
                      MemoryArena &arena, SurfaceInteraction *si,
                      Float *pdf) const;

	Spectrum sample_Sp(const Scene &scene, Float u1, const Point2f &u2,
                       MemoryArena &arena, SurfaceInteraction *si,
                       Float *pdf) const;

	Float pdf_Sp(const SurfaceInteraction &si) const;

	virtual Spectrum Sr(Float d) const = 0;

	virtual Float Sample_Sr(int ch, Float u) const = 0;

    virtual Float pdf_Sr(int ch, Float r) const = 0;

protected:
	// 着色法线
    const Normal3f _sNormal;
    // 着色切线(s方向，u方向)
    const Vector3f _sTangent;
    // 着色切线(t方向，v方向)
    const Vector3f _tTangent;
    // 材质
    const Material * _material;
    // 光照传输模式
    const TransportMode _mode;
	
};


/**
 * tabulated BSSRDF，在我理解为制成表格的bssrdf，该形式可以处理各种散射曲线
 * 包括实际测量的bssrdf
 * 
 * 先来介绍一下Sr函数
 * 注意：当材质属性固定时，Sr只是一个1D函数，更一般的来看Sr还有其他四个参数
 * 折射率η，散射各向异性系数g，反射率ρ，衰减系数σt，组合成一个完整的函数
 * Sr(η,g,ρ,σt,r)，不幸的是，这个函数对于离散化而言，维度太高了。
 * 上述参数中，除了r，只有σt是有物理单位的，这个参数量化了单位距离内的吸收率以及散射率
 * 这个参数比较简单，它仅仅控制了bssrdf的空间轮廓比例，为了降低函数的维度，我们可以把它固定为1
 * 并且制作一个无单位版本的bssrdf的外形轮廓。
 *
 * 在运行时，我们查找一个给定消光系数σt，半径r，我们找到一个无单位光学半径Roptical = σt r,
 * 计算更低维度的表格，表达式如下：
 * 
 *               Sr(η,g,ρ,σt,r) = σt^2 Sr(η,g,ρ,1,Roptical)
 *
 * 为何？因为Sr在极坐标(r,θ)空间中是一个2D密度函数，需要一个相应的比例因子来解释变量的变化
 *      p(r,θ) = r p(x,y)  (sampling.hpp文件8式)
 *
 * 
 *
 * 
 */
class TabulatedBSSRDF : public SeparableBSSRDF {
    
public:
    Spectrum Sr(Float distance) const;

private:
    
    
};


PALADIN_END

#endif /* bssrdf_hpp */
