//
//  texture.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef texture_hpp
#define texture_hpp

#include "header.h"
#include "math/transform.hpp"
#include "interaction.hpp"

PALADIN_BEGIN


/**
 * 我们要将纹理函数映射到物体的表面，有很多种方式
 * 最容易想到的就是uv映射，uv为物体表面参数坐标，st为纹理坐标，
 *
 * TextureMapping2D是2D纹理映射的基类，
 * 子类有uv映射，spherical，cylindrical，planar等
 * 目前我们先实现最简单的uv映射，剩下的内容后续补充，todo
 * 
 */
class TextureMapping2D {
public:
	virtual ~TextureMapping2D() {

	}
	
	/**
	 * 纹理映射函数
	 * @param  si    表面交点
	 * @param  dstdx 纹理坐标对屏幕坐标x的偏导数(两个分量分别对应ds/dx,dt/dx)
	 * @param  dstdy 纹理坐标对屏幕坐标y的偏导数(两个分量分别对应ds/dy,dt/dy)
	 * @return       返回映射之后的纹理坐标
	 */	
	virtual Point2f map(const SurfaceInteraction &si, Vector2f *dstdx, Vector2f *dstdy) const = 0;
};


/**
 * uv映射
 * 将传入的u,v坐标乘以一个系数再加上一个偏移量就得到了s,t坐标
 */
class UVMapping2D : public TextureMapping2D {
public:
	UVMapping2D(Float su = 1, Float sv = 1, Float du = 0, Float dv = 0);

    virtual Point2f map(const SurfaceInteraction &si, Vector2f *dstdx, Vector2f *dstdy)	const;

private:
	const Float _su, _sv, _du, _dv;
};

class SphericalMapping2D : public TextureMapping2D {
public:
	SphericalMapping2D(const Transform &worldToTexture)
	: _worldToTexture(worldToTexture) {

	}
	
	virtual Point2f map(const SurfaceInteraction &si, Vector2f *dstdx,
                Vector2f *dstdy) const;
private:
	Point2f pointToSphereToST(const Point3f &p) const;

	const Transform _worldToTexture;
};

/**
 * 纹理基类
 * 图像坐标为(x,y)，纹理坐标为(s,t)
 * 
 * 假设纹理函数为f
 * 我们采样一个纹理的值是不需要计算任意的f(x,y)的值
 * 我们只需要找到特定的点上，像素样本的变化与纹理样本变换的关系
 * 
 * f(x',y') ≈ f(x,y) + df/dx * (x' - x) + df/dy * (y' - y)
 * 
 */
template <typename T>
class Texture {
public:
    virtual T evaluate(const SurfaceInteraction &) const = 0;
    
    virtual ~Texture() {
        
    }
};

PALADIN_END

#endif /* texture_hpp */
