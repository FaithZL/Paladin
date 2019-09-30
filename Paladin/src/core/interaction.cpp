//
//  interaction.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/7/11.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "interaction.hpp"
#include "shape.hpp"

PALADIN_BEGIN

SurfaceInteraction::SurfaceInteraction(
                                       const Point3f &p, const Vector3f &pError, const Point2f &uv,
                                       const Vector3f &wo, const Vector3f &dpdu, const Vector3f &dpdv,
                                       const Normal3f &dndu, const Normal3f &dndv, Float time, const Shape *shape,
                                       int faceIndex)
: Interaction(p, Normal3f(normalize(cross(dpdu, dpdv))), pError, wo, time, nullptr),
uv(uv),
dpdu(dpdu),
dpdv(dpdv),
dndu(dndu),
dndv(dndv),
shape(shape),
faceIndex(faceIndex) {
    
    shading.normal = normal;
    shading.dpdu = dpdu;
    shading.dpdv = dpdv;
    shading.dndu = dndu;
    shading.dndv = dndv;
    
    if (shape &&
        (shape->reverseOrientation ^ shape->transformSwapsHandedness)) {
        normal *= -1;
        shading.normal *= -1;
    }
}

void SurfaceInteraction::computeDifferentials(const RayDifferential &ray) const {
    if (ray.hasDifferentials) {
        // 平面方程为 ax + by + cz = d
        // 法向量为n(a,b,c),平面上的点p(x,y,z)
        // d = n · p
        // 已知平面方程，射线参数，求交点，表达式如下
        // a(ox + t dirX) + b(oy + t dirX) + c(oz + t dirZ) = d
        // 整理得t = (d - (a,b,c) · o) / (a,b,c) · dir
        Float d = dot(normal, Vector3f(pos));
        Float tx = (d - dot(normal, Vector3f(ray.rxOrigin))) / dot(normal, ray.rxDirection);
        if (std::isinf(tx) || std::isnan(tx)) {
            goto fail;
        }
        Point3f px = ray.rxOrigin + tx * ray.rxDirection;
        Float ty = (d - dot(normal, Vector3f(ray.ryOrigin))) / dot(normal, ray.ryDirection);
        Point3f py = ray.ryOrigin + ty * ray.ryDirection;

        dpdx = px - pos;
        dpdy = py - pos;

        /**
         * p' = p + △u dp/du + △v dp/dv
         *  
         *  展开之后可以转化成矩阵相乘的形式
         *
         *  p'x = px + △u dpx/du + △v dpx/dv
         *  p'y = py + △u dpy/du + △v dpy/dv
         *  p'z = pz + △u dpz/du + △v dpz/dv
         *          
         * | p'x - px |   | dpx/du dpx/dv |   | △u |
         * | p'y - py | = | dpy/du dpy/dv | * |    |
         * | p'z - pz |   | dpz/du dpz/dv |   | △v |
         * 
         * 以上形式，有两个未知数，但有三个方程，显然不符合我们的数学常识(过度约束了)
         * 也许这个方程组可以退化为二元一次方程组
         * 例如，假设dp/du,dp/dv都在xy平面上，dpz/du与dpz/dv都为0
         * 因为点p的xyz坐标之间是有约束条件的，所以！
         * 我们可以确定以上方程组可以退化为二元一次方程组
         *
         * 现在需要选择其中两个方程去求解，选择哪两个呢？
         * dp/du,dp/dv叉乘得到法向量normal，找到normal最大的一个分量
         * 用较小两个分量求解线性方程组
         * 
         * btw: 为何要这样选择？
         * 假设normal为(0,0,1)，这时如果选择的是x,z两个维度，或者y，z
         * 这特么显然是算不出来的嘛，uv变化都不会引起z变化，还算个鸡毛！
         * 所以要选择normal较小的两个维度，确保这p点的这两个维度随着uv变化而都会发生变化
         *
         * y轴方向的辅助光线微分计算同理，不再赘述
         */
        int dim[2];
        if (std::abs(normal.x) > std::abs(normal.y) 
            && std::abs(normal.x) > std::abs(normal.z)) {
            dim[0] = 1;
            dim[1] = 2;
        } else if (std::abs(normal.y) > std::abs(normal.z)) {
            dim[0] = 0;
            dim[1] = 2;
        } else {
            dim[0] = 0;
            dim[1] = 1;
        }
        Float A[2][2] = {{dpdu[dim[0]], dpdv[dim[0]]},
                         {dpdu[dim[1]], dpdv[dim[1]]}};
        Float Bx[2] = {px[dim[0]] - pos[dim[0]], px[dim[1]] - pos[dim[1]]};
        Float By[2] = {py[dim[0]] - pos[dim[0]], py[dim[1]] - pos[dim[1]]};
        if (!solveLinearSystem2x2(A, Bx, &dudx, &dvdx)) {
            dudx = dvdx = 0;
        }
        if (!solveLinearSystem2x2(A, By, &dudy, &dvdy)) {
            dudy = dvdy = 0;        
        }

    } else {
        fail:
        dudx = dvdx = 0;
        dudy = dvdy = 0;
        dpdx = dpdy = Vector3f(0, 0, 0);
    }
}

void SurfaceInteraction::setShadingGeometry(const Vector3f &dpdus,
                                            const Vector3f &dpdvs,
                                            const Normal3f &dndus,
                                            const Normal3f &dndvs,
                                            bool orientationIsAuthoritative) {

    shading.normal = normalize((Normal3f)cross(dpdus, dpdvs));
    
    if (shape && (shape->reverseOrientation ^ shape->transformSwapsHandedness)) {
        shading.normal = -shading.normal;
    }

    if (orientationIsAuthoritative) {
        normal = faceforward(normal, shading.normal);
    } else {
        shading.normal = faceforward(shading.normal, normal);
    }

    shading.dpdu = dpdus;
    shading.dpdv = dpdvs;
    shading.dndu = dndus;
    shading.dndv = dndvs;
}

PALADIN_END
