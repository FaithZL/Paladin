//
//  sphere.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "shapes/sphere.hpp"
#include "math/sampling.hpp"

PALADIN_BEGIN

bool Sphere::intersect(const Ray &r, Float *tHit, SurfaceInteraction *isect, bool testAlphaTexture) const {
    Float phi;
    Point3f pHit;
    Vector3f oErr, dErr;
    
    Ray ray = worldToObject->exec(r, &oErr, &dErr);
    // (ox + t * dx^2) + (oy + t * dy^2) + (oz + t * dz^2) = r^2.
    // a * t^2 + b * t + c = 0，则a，b，c如下所示
    EFloat ox(ray.ori.x, oErr.x), oy(ray.ori.y, oErr.y), oz(ray.ori.z, oErr.z);
    EFloat dx(ray.dir.x, dErr.x), dy(ray.dir.y, dErr.y), dz(ray.dir.z, dErr.z);
    EFloat a = dx * dx + dy * dy + dz * dz;
    EFloat b = 2 * (dx * ox + dy * oy + dz * oz);
    EFloat c = ox * ox + oy * oy + oz * oz - EFloat(_radius) * EFloat(_radius);
    
    EFloat t0, t1;
    if (!quadratic(a, b, c, &t0, &t1)) {
        return false;
    }

    // 保守计算
    if (t0.upperBound() > ray.tMax || t1.lowerBound() <= 0) {
         return false;
    }
    EFloat tShapeHit = t0;

    if (tShapeHit.lowerBound() <= 0) {
        tShapeHit = t1;
        if (tShapeHit.upperBound() > ray.tMax) {
            return false;
        }
    }

    pHit = ray.at((Float)tShapeHit);
    pHit *= _radius / distance(pHit, Point3f(0, 0, 0));

    if (pHit.x == 0 && pHit.y == 0) {
        // 避免0/0
        pHit.x = 1e-5f * _radius;
    }

    phi = std::atan2(pHit.y, pHit.x);
    if (phi < 0) {
        phi += 2 * Pi;
    }

    // 测试球体的裁剪参数
    if ((_zMin > -_radius && pHit.z < _zMin) 
        || (_zMax < _radius && pHit.z > _zMax) 
        || phi > _phiMax) {
        // 如果pHit处在球的缺失部分
        if (tShapeHit == t1) {
            // 如果tShapeHit已经是较远点，则返回false
            return false;
        }
        if (t1.upperBound() > ray.tMax) {
            return false;
        }

        // 此时的逻辑是t0处于球的缺失部分，则开始判断t1
        tShapeHit = t1;
        pHit = ray.at((Float)tShapeHit);

        pHit *= _radius / distance(pHit, Point3f(0, 0, 0));
        if (pHit.x == 0 && pHit.y == 0) {
            // 避免0/0
            pHit.x = 1e-5f * _radius;
        }

        phi = std::atan2(pHit.y, pHit.x);
        if (phi < 0) {
            phi += 2 * Pi;
        }

        // 再次测试球体裁剪参数
        if ((_zMin > -_radius && pHit.z < _zMin) 
            || (_zMax < _radius && pHit.z > _zMax) 
            || phi > _phiMax) {
            return false;
        }

    }

    // φ = u * φmax
    // θ = θmin + v * (θmax - θmin)
    Float u = phi / _phiMax;
    Float theta = std::acos(clamp(pHit.z / _radius, -1, 1));
    Float v = (theta - _thetaMin) / (_thetaMax - _thetaMin);

    Float zRadius = std::sqrt(pHit.x * pHit.x + pHit.y * pHit.y);
    Float invZRadius = 1 / zRadius;
    Float cosPhi = pHit.x * invZRadius;
    Float sinPhi = pHit.y * invZRadius;

    // 曲面上的点p对参数uv的一阶偏导数，以及二阶偏导数，直接用高中数学知识求出即可，具体推导过程就不写了
    Vector3f dpdu(-_phiMax * pHit.y, _phiMax * pHit.x, 0);
    Vector3f dpdv = (_thetaMax - _thetaMin) *
        Vector3f(pHit.z * cosPhi, pHit.z * sinPhi, -_radius * std::sin(theta));

    Vector3f d2Pduu = -_phiMax * _phiMax * Vector3f(pHit.x, pHit.y, 0);
    Vector3f d2Pduv = (_thetaMax - _thetaMin) 
                        * pHit.z * _phiMax
                        * Vector3f(-sinPhi, cosPhi, 0.);
    Vector3f d2Pdvv = -(_thetaMax - _thetaMin)
                        * (_thetaMax - _thetaMin) 
                        * Vector3f(pHit.x, pHit.y, pHit.z);

    // 法线对于uv参数的一阶二阶偏导数就略显复杂
    Float E = dot(dpdu, dpdu);
    Float F = dot(dpdu, dpdv);
    Float G = dot(dpdv, dpdv);
    Vector3f N = normalize(cross(dpdu, dpdv));
    Float e = dot(N, d2Pduu);
    Float f = dot(N, d2Pduv);
    Float g = dot(N, d2Pdvv);   

    Float invEGF2 = 1 / (E * G - F * F);
    Normal3f dndu = Normal3f((f * F - e * G) * invEGF2 * dpdu +
                             (e * F - f * E) * invEGF2 * dpdv);
    Normal3f dndv = Normal3f((g * F - f * G) * invEGF2 * dpdu +
                             (f * F - g * E) * invEGF2 * dpdv);

    // sampleA函数中有gamma(5)的推导过程 为何不能像圆锥那样求误差？todo
    Vector3f pError = gamma(5) * abs(Vector3f(pHit));

    *isect = objectToWorld->exec(SurfaceInteraction(pHit, pError, Point2f(u, v),
                                                 -ray.dir, dpdu, dpdv, dndu, dndv,
                                                    ray.time, this));
    
    *tHit = (Float)tShapeHit;
    return true;
}


bool Sphere::intersectP(const Ray &r, bool testAlphaTexture) const {
    Float phi;
    Point3f pHit;
    Vector3f oErr, dErr;
    
    Ray ray = worldToObject->exec(r, &oErr, &dErr);
    // (ox + t * dx^2) + (oy + t * dy^2) + (oz + t * dz^2) = r^2.
    // a * t^2 + b * t + c = 0，则a，b，c如下所示
    EFloat ox(ray.ori.x, oErr.x), oy(ray.ori.y, oErr.y), oz(ray.ori.z, oErr.z);
    EFloat dx(ray.dir.x, dErr.x), dy(ray.dir.y, dErr.y), dz(ray.dir.z, dErr.z);
    EFloat a = dx * dx + dy * dy + dz * dz;
    EFloat b = 2 * (dx * ox + dy * oy + dz * oz);
    EFloat c = ox * ox + oy * oy + oz * oz - EFloat(_radius) * EFloat(_radius);
    
    EFloat t0, t1;
    if (!quadratic(a, b, c, &t0, &t1)) {
        return false;
    }
    
    // 保守计算
    if (t0.upperBound() > ray.tMax || t1.lowerBound() <= 0) {
         return false;
    }
    EFloat tShapeHit = t0;

    if (tShapeHit.lowerBound() <= 0) {
        tShapeHit = t1;
        if (tShapeHit.upperBound() > ray.tMax) {
            return false;
        }
    }

    pHit = ray.at((Float)tShapeHit);
    pHit *= _radius / distance(pHit, Point3f(0, 0, 0));

    if (pHit.x == 0 && pHit.y == 0) {
        // 避免0/0
        pHit.x = 1e-5f * _radius;
    }

    phi = std::atan2(pHit.y, pHit.x);
    if (phi < 0) {
        phi += 2 * Pi;
    }

    // 测试球体的裁剪参数
    if ((_zMin > -_radius && pHit.z < _zMin) 
        || (_zMax < _radius && pHit.z > _zMax) 
        || phi > _phiMax) {
        // 如果pHit处在球的缺失部分
        if (tShapeHit == t1) {
            // 如果tShapeHit已经是较远点，则返回false
            return false;
        }
        if (t1.upperBound() > ray.tMax) {
            return false;
        }

        // 此时的逻辑是t0处于球的缺失部分，则开始判断t1
        tShapeHit = t1;
        pHit = ray.at((Float)tShapeHit);

        pHit *= _radius / distance(pHit, Point3f(0, 0, 0));
        if (pHit.x == 0 && pHit.y == 0) {
            // 避免0/0
            pHit.x = 1e-5f * _radius;
        }

        phi = std::atan2(pHit.y, pHit.x);
        if (phi < 0) {
            phi += 2 * Pi;
        }

        // 再次测试球体裁剪参数
        if ((_zMin > -_radius && pHit.z < _zMin) 
            || (_zMax < _radius && pHit.z > _zMax) 
            || phi > _phiMax) {
            return false;
        }

    }
    
    return true;
}

Interaction Sphere::sampleA(const Point2f &u, Float *pdf) const {
    Point3f pObj = Point3f(0, 0, 0) + _radius * uniformSampleSphere(u);
    Interaction ret;
    ret.normal = normalize(objectToWorld->exec(Normal3f(pObj.x, pObj.y, pObj.z)));
    if (reverseOrientation) {
        ret.normal *= -1;
    }
    // 由于uniformSampleSphere函数使用了cos与sin函数
    // pObj的误差取决于这些函数的精度，所以pObj需要被重新投影到球面上
    pObj *= _radius / distance(pObj, Point3f(0, 0, 0));
    
    /**
     * x' = x * (r / sqrt(x^2 + y^2 + z^2))
     * x' = x ⊗ r ⊘ sqrt((x ⊗ x) ⊕ (y ⊗ y) ⊕ (z ⊗ z))
     * ∈ xr(1± ε)^2 / sqrt(x^2*(1 ± ε)^3 + y^2*(1 ± ε)^3 + z^2*(1 ± ε)^2)(1 ± ε)
     * ∈ xr(1± γ2) / sqrt(x^2*(1 ± γ3) + y^2*(1 ± γ3) + z^2*(1 ± γ2))(1 ± γ1)
     * 个人理解，这里是扩大范围保守估计γ3变成了γ4，开方之后得γ2
     * ∈ xr(1± γ2) / sqrt((x^2 + y^2 + z^2)*(1 ± γ4))(1 ± γ1)
     * 开方之后得γ2
     * ∈ xr(1± γ2) / sqrt((x^2 + y^2 + z^2))(1 ± γ2)(1 ± γ1)
     * 保守估计，两个下标相加得5
     * ∈ (xr / sqrt(x2 + y2 + z2)) * (1 ± γ5)
     * y与z同理
     */
    Vector3f pObjError = gamma(5) * abs(Vector3f(pObj));
    ret.pos = objectToWorld->exec(pObj, pObjError, &ret.pError);
    *pdf = pdfA(ret);
    return ret;
}

Interaction Sphere::sampleW(const paladin::Interaction &ref, const Point2f &u, Float *pdf) const {
    Interaction ret;
    Point3f pCenter = objectToWorld->exec(Point3f(0,0,0));
    Point3f pOrigin = offsetRayOrigin(ref.pos, ref.pError, ref.normal, pCenter - ref.pos);
    if (distanceSquared(pOrigin, pCenter) < _radius * _radius) {
        // ref点在球内
        // p(w) = r^2 / cosθ * p(A)
        Float pdfA = 0;
        ret = sampleA(u, &pdfA);
        Vector3f wi = ret.pos - ref.pos;
        if (wi.lengthSquared() == 0) {
            *pdf = 0;
        } else {
            Float cosTheta = absDot(normalize(-wi), ret.normal);
            *pdf = (wi.lengthSquared() * pdfA) / cosTheta;
        }
        return ret;
    }
    // ref在球外，为圆锥采样的，公式推导参见sampling.hpp文件uniformSampleCone
    // ref到圆心的距离dc
    Float dc = distance(pCenter, ref.pos);
    Float invDc = 1 / dc;
    // 构建一个坐标系
    Vector3f wc = (pCenter - ref.pos) * invDc;
    Vector3f wcX, wcY;
    coordinateSystem(wc, &wcX, &wcY);
    
    Float sinThetaMax = _radius * invDc;
    Float sinThetaMax2 = sinThetaMax * sinThetaMax;
    Float cosThetaMax = std::sqrt(1 - sinThetaMax2);
    
    Float cosTheta  = (cosThetaMax - 1) * u[0] + 1;
    Float sinTheta = std::sqrt(std::max((Float)0 , 1 - cosTheta * cosTheta));
    Float phi = u[1] * 2 * Pi;
    //角α为-wc向量与球面采样点与圆心连线的夹角
    Float ds = dc * cosTheta -
            std::sqrt(std::max((Float)0,
                    _radius * _radius - dc * dc * sinTheta * sinTheta));
    // 余弦定理求出cosα
    Float cosAlpha = (dc * dc + _radius * _radius - ds * ds) / (2 * dc * _radius);
    Float sinAlpha = std::sqrt(std::max((Float)0, 1 - cosAlpha * cosAlpha));
    // 将圆锥采样得到的theta与phi转换成在世界空间的向量
    Vector3f nWorld = sphericalDirection(sinAlpha, cosAlpha, phi, -wcX, -wcY, -wc);
    Point3f pWorld = pCenter + _radius * Point3f(nWorld.x, nWorld.y, nWorld.z);
    
    ret.pos = pWorld;
    ret.pError = gamma(5) * abs(Vector3f(pWorld));
    ret.normal = Normal3f(nWorld);
    *pdf = uniformConePdf(cosThetaMax);
    
    return ret;
}

Float Sphere::pdfW(const paladin::Interaction &ref, const Vector3f &wi) const {
    Point3f pCenter = objectToWorld->exec(Point3f(0,0,0));
    // Point3f origin = ref.pos; todo
    Point3f origin = offsetRayOrigin(ref.pos, ref.pError, ref.normal, wi);
    if (distanceSquared(pCenter, origin) <= _radius * _radius) {
        // 如果在球内
        return Shape::pdfW(ref, wi);
    }
    
    // 如果在球外，则进行圆锥采样
    Float sinThetaMax2 = _radius * _radius / distanceSquared(ref.pos, pCenter);
    Float cosThetaMax = std::sqrt(std::max((Float)0, 1 - sinThetaMax2));
    return uniformConePdf(cosThetaMax);
}

PALADIN_END
