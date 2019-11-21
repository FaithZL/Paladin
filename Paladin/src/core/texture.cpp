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
	
    
    // 正向差分法会带来一个问题
    // 考虑φ在0和2π之间的间断点情况
    // 如果φ在2π处，此时t = 1，φ进行正向偏移△之后，此时t = △/(2π)，接近于0
    // 这显然不是我们希望看到的
    // 如何解决这个问题？
    // 先说结论，dt/dx不可能大于0.5，如果dt/dx大于0.5，一定是出现了跳跃点
    // 为何？？？
    // 我们把屏幕空间的dx定为单位1，光线经过dx偏移之后，
    // φ所对应的偏移值一定是小于π的，所以t也是小于0.5的
    // 因为球是圆的嘛，光线偏移也不可能照到球的背面去
    // 接下来的圆柱映射也同理

	if ((*dstdx)[1] > 0.5f) {
		// 0到2π
        (*dstdx)[1] = 1 - (*dstdx)[1];
	} else if ((*dstdx)[1] < -0.5f) {
		// 2π到0
        (*dstdx)[1] = -((*dstdx)[1] + 1);
    } 

	Point2f stDeltaY = pointToSphereToST(si.pos + delta * si.dpdy);
    *dstdy = (stDeltaY - st) / delta;    
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

// CylindricalMapping2D
Point2f CylindricalMapping2D::map(const SurfaceInteraction &si, Vector2f *dstdx,
                                  Vector2f *dstdy) const {
	// 这个版本跟SphericalMapping2D的没太大区别，
	// 用同样的方式处理正向差分产生的问题
    Point2f st = pointToCylinderToST(si.pos);

    const Float delta = .01f;
    Point2f stDeltaX = pointToCylinderToST(si.pos + delta * si.dpdx);
    *dstdx = (stDeltaX - st) / delta;
    if ((*dstdx)[1] > .5) {
        (*dstdx)[1] = 1.f - (*dstdx)[1];
    } else if ((*dstdx)[1] < -.5f) {
        (*dstdx)[1] = -((*dstdx)[1] + 1);
    }

    Point2f stDeltaY = pointToCylinderToST(si.pos + delta * si.dpdy);
    *dstdy = (stDeltaY - st) / delta;
    if ((*dstdy)[1] > .5) {
        (*dstdy)[1] = 1.f - (*dstdy)[1];
    } else if ((*dstdy)[1] < -.5f) {
        (*dstdy)[1] = -((*dstdy)[1] + 1);
    }
    return st;
}

// PlanarMapping2D
Point2f PlanarMapping2D::map(const SurfaceInteraction &si, Vector2f *dstdx,
                             Vector2f *dstdy) const {
    Vector3f vec(si.pos);
    // 由向量函数求导的链式法则可得
    // ds/dx = ds/dp dp/dx
    // 
    // s = vec · _vs + _ds
    // s = vec.x * _vs.x + vec.y * _vs.y + vec.z * _vs.z + _ds
    // 
    // 			 ds/dp的雅克比矩阵     dp/dx的雅克比矩阵
    // 			  
    //                                 | (dp/dx)x |
    // ds/dx = |_vs.x, _vs.y, _vs.z| * | (dp/dx)y |
    //                                 | (dp/dx)z |
    // 
    // ∴ ds/dx 数值上与 dp/dx 点乘 _vs 相等。
    // 在标量对标量的导函数也一定是标量，确实可以解释上面的表达式
    *dstdx = Vector2f(dot(si.dpdx, _vs), dot(si.dpdx, _vt));
    *dstdy = Vector2f(dot(si.dpdy, _vs), dot(si.dpdy, _vt));
    return Point2f(_ds + dot(vec, _vs), _dt + dot(vec, _vt));
}

// IdentityMapping3D
Point3f IdentityMapping3D::map(const SurfaceInteraction &si, Vector3f *dpdx,
                               Vector3f *dpdy) const {
    *dpdx = _worldToTexture.exec(si.dpdx);
    *dpdy = _worldToTexture.exec(si.dpdy);
    return _worldToTexture.exec(si.pos);
}

Float lanczos(Float x, Float tau) {
    x = std::abs(x);
    if (x < 1e-5f) {
        return 1;
    }
    if (x > 1.f) {
        return 0;
    }
    x *= Pi;
    Float s = std::sin(x * tau) / (x * tau);
    Float lanczos = std::sin(x) / x;
    return s * lanczos;
}

//"data" : {
//    "type" : "constant",
//    "param" : 0
//}
Texture<Float> * createTextureFloat(const nloJson &data) {
    string fullType = data.value("type", "constant") + "Float";
    auto creator = GET_CREATOR(fullType);
    nloJson param = data.value("param", nloJson::array({}));
    CObject_ptr tmp = creator(param, {});
    return dynamic_cast<Texture<Float> *>(tmp);
}

//"data" : {
//    "type" : "constant",
//    "param" : {
//        "colorType" : 0,
//        "color" : [0.1, 0.9, 0.5],
//    }
//}
Texture<Spectrum> * createTextureSpectrum(const nloJson &data) {
    string fullType = data.value("type", "constant") + "Spectrum";
    auto creator = GET_CREATOR(fullType);
    nloJson param = data.value("param", nloJson::array({}));
    CObject_ptr tmp = creator(param, {});
    return dynamic_cast<Texture<Spectrum> *>(tmp);
}

PALADIN_END
