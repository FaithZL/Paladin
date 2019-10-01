//
//  texture.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "texture.hpp"


PALADIN_BEGIN

UVMapping2D::UVMapping2D(Float su, Float sv, Float du, Float dv)
:_su(su),
_sv(sv),
_du(du),
_dv(dv) {
    
}

Point2f UVMapping2D::map(const paladin::SurfaceInteraction &si, Vector2f *dstdx, Vector2f *dstdy) const {
    // 求s对x的偏导数ds/dx
    // 由导函数链式法则，ds/dx = ds/du * du/dx
    // 显然偏导数ds/du = _su，则可以写出如下表达式
    Float dsdx = _su * si.dudx;

    // dt/dx = dt/dv * dv/dx
    Float dtdx = _sv * si.dvdx;
    *dstdx = Vector2f(dsdx, dtdx);

    // 同理可求出ds/dy，dt/dy
    Float dsdy = _su * si.dudy;
    Float dtdy = _sv * si.dvdy;
    *dstdy = Vector2f(dsdy, dtdy);

    Float s = _su * si.uv[0] + _du;
    Float t = _sv * si.uv[1] + _dv;
    return Point2f(s, t);
}

//SphericalMapping2D
Point2f SphericalMapping2D::map(const paladin::SurfaceInteraction &si, Vector2f *dstdx, Vector2f *dstdy) const {
    Point2f st = pointToSphereToST(si.pos);

    // 偏导数，用正向差分法(其实就是导函数的定义)
    // fs为点p映射到s的函数
    //  ds        fs(p + △x dp/dx) - fs(p)
    // ---- ≈ ---------------------------------
    //  dx                 △x
    //  得到如下表达式
    const Float delta = 0.1f;
    Point2f stDeltaX = pointToSphereToST(si.pos + delta * si.dpdx);
    *dstdx = (stDeltaX - st) / delta;
	Point2f stDeltaY = pointToSphereToST(si.pos + delta * si.dpdy);
    *dstdy = (stDeltaY - st) / delta;
    
    // 正向差分法会带来一个问题
    // 考虑φ在0和2π之间的间断点情况
    // 如果φ在2π处，此时t = 1，φ进行正向偏移△之后，此时t = △/(2π)，接近于0
    // 这显然不是我们希望看到的
    // 这是pbrt代码，我觉得有点问题，先标注上
    // 
    // s经过正向偏移之后得到s'，s' 与 s 均属于 [0,1] 范围
    // 
    // 我们暂时把 
    // 值接近1的s值，正向偏移之后，s‘略大于1，重新映射到0到1之间，这种情况称为越界
    // 
    // 如果不考虑越界s'越界的情况，s'比s略大一点点，两者差值应该很小
    // 
    // 如果考虑越界情况 s略小于1，s'略大于0，两者差值接近1,
    // 除以delta之后，应该是接近10，
    // 所以判断是否越界的标准应该是 偏导数是否大于5
    // 
	if ((*dstdx)[1] > 0.5f) {
		// 
        (*dstdx)[1] = 1 - (*dstdx)[1];
	} else if ((*dstdx)[1] < -0.5f) {
        (*dstdx)[1] = -((*dstdx)[1] + 1);
    } 

    if ((*dstdy)[1] > 0.5f) {
        (*dstdy)[1] = 1 - (*dstdy)[1];
    } else if ((*dstdy)[1] < -0.5f) {
        (*dstdy)[1] = -((*dstdy)[1] + 1);
    }

    return st;
}

Point2f SphericalMapping2D::pointToSphereToST(const Point3f &p) const {
    Vector3f vec = normalize(_worldToTexture.exec(p) - Point3f(0, 0, 0));
    Float theta = sphericalTheta(vec);
    Float phi = sphericalPhi(vec);
    return Point2f(theta * InvPi, phi * Inv2Pi);
}

PALADIN_END
