//
//  triangle.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "shapes/trianglemesh.hpp"
#include "math/sampling.hpp"
#include "lights/diffuse.hpp"
#include "core/primitive.hpp"
#include "parser/modelparser.hpp"

PALADIN_BEGIN

TriangleMesh::TriangleMesh(
                           const shared_ptr<const Transform> &ObjectToWorld, int nTriangles, const int *vertexIndices,
                           int nVertices, const Point3f *P, const Vector3f *S, const Normal3f *N,
                           const Point2f *UV, const std::shared_ptr<Texture<Float>> &alphaMask,
                           const std::shared_ptr<Texture<Float>> &shadowAlphaMask,
                           const int *fIndices)
: nTriangles(nTriangles),
nVertices(nVertices),
vertexIndices(vertexIndices, vertexIndices + 3 * nTriangles),
alphaMask(alphaMask),
shadowAlphaMask(shadowAlphaMask) {

    
    points.reset(new Point3f[nVertices]);
    for (int i = 0; i < nVertices; ++i) {
        Point3f p = P[i];
        points[i] = ObjectToWorld->exec(p);
    }
    
    if (UV) {
        uv.reset(new Point2f[nVertices]);
        memcpy(uv.get(), UV, nVertices * sizeof(Point2f));
    }
    if (N) {
        normals.reset(new Normal3f[nVertices]);
        for (int i = 0; i < nVertices; ++i) {
            normals[i] = ObjectToWorld->exec(N[i]);
        }
    }
    if (S) {
        edges.reset(new Vector3f[nVertices]);
        for (int i = 0; i < nVertices; ++i) {
            edges[i] = ObjectToWorld->exec(S[i]);
        }
    }
    
    if (fIndices) {
        faceIndices = std::vector<int>(fIndices, fIndices + nTriangles);
    }
}

AABB3f Triangle::objectBound() const {
    const Point3f &p0 = _mesh->points[_vertexIdx[0]];
    const Point3f &p1 = _mesh->points[_vertexIdx[1]];
    const Point3f &p2 = _mesh->points[_vertexIdx[2]];
    AABB3f b1 = AABB3f(worldToObject->exec(p0), worldToObject->exec(p1));
    return unionSet(b1, worldToObject->exec(p2));;
}

bool Triangle::watertightIntersect(const Ray &ray, Float *tHit, SurfaceInteraction *isect,
                         bool testAlphaTexture) const {
    const Point3f &p0 = _mesh->points[_vertexIdx[0]];
    const Point3f &p1 = _mesh->points[_vertexIdx[1]];
    const Point3f &p2 = _mesh->points[_vertexIdx[2]];
    
    Point3f p0t = p0 - Vector3f(ray.ori);
    Point3f p1t = p1 - Vector3f(ray.ori);
    Point3f p2t = p2 - Vector3f(ray.ori);

     // Permute components of triangle vertices and ray direction
    int kz = maxDimension(abs(ray.dir));
    int kx = kz + 1;
    if (kx == 3) kx = 0;
    int ky = kx + 1;
    if (ky == 3) ky = 0;
    Vector3f d = permute(ray.dir, kx, ky, kz);
    p0t = permute(p0t, kx, ky, kz);
    p1t = permute(p1t, kx, ky, kz);
    p2t = permute(p2t, kx, ky, kz);
    // Apply shear transformation to translated vertex positions
    Float Sx = -d.x / d.z;
    Float Sy = -d.y / d.z;
    Float Sz = 1.f / d.z;
    p0t.x += Sx * p0t.z;
    p0t.y += Sy * p0t.z;
    p1t.x += Sx * p1t.z;
    p1t.y += Sy * p1t.z;
    p2t.x += Sx * p2t.z;
    p2t.y += Sy * p2t.z;

     // Compute edge function coefficients _e0_, _e1_, and _e2_
    Float e0 = p1t.x * p2t.y - p1t.y * p2t.x;
    Float e1 = p2t.x * p0t.y - p2t.y * p0t.x;
    Float e2 = p0t.x * p1t.y - p0t.y * p1t.x;

     // Fall back to double precision test at triangle edges
    if (sizeof(Float) == sizeof(float) &&
        (e0 == 0.0f || e1 == 0.0f || e2 == 0.0f)) {
        double p2txp1ty = (double)p2t.x * (double)p1t.y;
        double p2typ1tx = (double)p2t.y * (double)p1t.x;
        e0 = (float)(p2typ1tx - p2txp1ty);
        double p0txp2ty = (double)p0t.x * (double)p2t.y;
        double p0typ2tx = (double)p0t.y * (double)p2t.x;
        e1 = (float)(p0typ2tx - p0txp2ty);
        double p1txp0ty = (double)p1t.x * (double)p0t.y;
        double p1typ0tx = (double)p1t.y * (double)p0t.x;
        e2 = (float)(p1typ0tx - p1txp0ty);
    }

     // Perform triangle edge and determinant tests
    if ((e0 < 0 || e1 < 0 || e2 < 0) && (e0 > 0 || e1 > 0 || e2 > 0))
        return false;
    Float det = e0 + e1 + e2;
    if (det == 0) return false;

    // Compute scaled hit distance to triangle and test against ray $t$ range
    p0t.z *= Sz;
    p1t.z *= Sz;
    p2t.z *= Sz;
    Float tScaled = e0 * p0t.z + e1 * p1t.z + e2 * p2t.z;
    if (det < 0 && (tScaled >= 0 || tScaled < ray.tMax * det))
        return false;
    else if (det > 0 && (tScaled <= 0 || tScaled > ray.tMax * det))
        return false;

     // Compute barycentric coordinates and $t$ value for triangle intersection
    Float invDet = 1 / det;
    Float b0 = e0 * invDet;
    Float b1 = e1 * invDet;
    Float b2 = e2 * invDet;
    Float t = tScaled * invDet;

    // Ensure that computed triangle $t$ is conservatively greater than zero

    // Compute $\delta_z$ term for triangle $t$ error bounds
    Float maxZt = maxComponent(abs(Vector3f(p0t.z, p1t.z, p2t.z)));
    Float deltaZ = gamma(3) * maxZt;

    // Compute $\delta_x$ and $\delta_y$ terms for triangle $t$ error bounds
    Float maxXt = maxComponent(abs(Vector3f(p0t.x, p1t.x, p2t.x)));
    Float maxYt = maxComponent(abs(Vector3f(p0t.y, p1t.y, p2t.y)));
    Float deltaX = gamma(5) * (maxXt + maxZt);
    Float deltaY = gamma(5) * (maxYt + maxZt);

    // Compute $\delta_e$ term for triangle $t$ error bounds
    Float deltaE =
         2 * (gamma(2) * maxXt * maxYt + deltaY * maxXt + deltaX * maxYt);

     // Compute $\delta_t$ term for triangle $t$ error bounds and check _t_
    Float maxE = maxComponent(abs(Vector3f(e0, e1, e2)));
    Float deltaT = 3 *
                (gamma(3) * maxE * maxZt + deltaE * maxZt + deltaZ * maxE) *
                std::abs(invDet);
    if (t <= deltaT) return false;

    // Compute triangle partial derivatives
    Vector3f dpdu, dpdv;
    Point2f uv[3];
    getUVs(uv);

    // Compute deltas for triangle partial derivatives
    Vector2f duv02 = uv[0] - uv[2], duv12 = uv[1] - uv[2];
    Vector3f dp02 = p0 - p2, dp12 = p1 - p2;
    Float determinant = duv02[0] * duv12[1] - duv02[1] * duv12[0];
    bool degenerateUV = std::abs(determinant) < 1e-8;
    if (!degenerateUV) {
        Float invdet = 1 / determinant;
        dpdu = (duv12[1] * dp02 - duv02[1] * dp12) * invdet;
        dpdv = (-duv12[0] * dp02 + duv02[0] * dp12) * invdet;
    }
    if (degenerateUV || cross(dpdu, dpdv).lengthSquared() == 0) {
        // Handle zero determinant for triangle partial derivative matrix
        Vector3f ng = cross(p2 - p0, p1 - p0);
        if (ng.lengthSquared() == 0)
            // The triangle is actually degenerate; the intersection is
            // bogus.
            return false;

        coordinateSystem(normalize(ng), &dpdu, &dpdv);
    }

     // Compute error bounds for triangle intersection
    Float xAbsSum =
         (std::abs(b0 * p0.x) + std::abs(b1 * p1.x) + std::abs(b2 * p2.x));
    Float yAbsSum =
         (std::abs(b0 * p0.y) + std::abs(b1 * p1.y) + std::abs(b2 * p2.y));
    Float zAbsSum =
         (std::abs(b0 * p0.z) + std::abs(b1 * p1.z) + std::abs(b2 * p2.z));
    Vector3f pError = gamma(7) * Vector3f(xAbsSum, yAbsSum, zAbsSum);

    // Interpolate $(u,v)$ parametric coordinates and hit point
    Point3f pHit = b0 * p0 + b1 * p1 + b2 * p2;
    Point2f uvHit = b0 * uv[0] + b1 * uv[1] + b2 * uv[2];

    // Test intersection against alpha texture, if present
    if (testAlphaTexture && _mesh->alphaMask) {
        SurfaceInteraction isectLocal(pHit, Vector3f(0, 0, 0), uvHit, -ray.dir,
                                       dpdu, dpdv, Normal3f(0, 0, 0),
                                       Normal3f(0, 0, 0), ray.time, this);
        if (_mesh->alphaMask->evaluate(isectLocal) == 0) return false;
    }

     // Fill in _SurfaceInteraction_ from triangle hit
    *isect = SurfaceInteraction(pHit, pError, uvHit, -ray.dir, dpdu, dpdv,
                                 Normal3f(0, 0, 0), Normal3f(0, 0, 0), ray.time,
                                 this, _faceIndex);

     // Override surface normal in _isect_ for triangle
    isect->normal = isect->shading.normal = Normal3f(normalize(cross(dp02, dp12)));
    if (_mesh->normals || _mesh->edges) {
        // Initialize _Triangle_ shading geometry

        // Compute shading normal _ns_ for triangle
        Normal3f ns;
        if (_mesh->normals) {
            ns = (b0 * _mesh->normals[_vertexIdx[0]] + b1 * _mesh->normals[_vertexIdx[1]] + b2 * _mesh->normals[_vertexIdx[2]]);
            if (ns.lengthSquared() > 0)
                ns = normalize(ns);
            else
                ns = isect->normal;
        } else
             ns = isect->normal;

         // Compute shading tangent _ss_ for triangle
        Vector3f ss;
        if (_mesh->edges) {
            ss = (b0 * _mesh->edges[_vertexIdx[0]] + b1 * _mesh->edges[_vertexIdx[1]] + b2 * _mesh->edges[_vertexIdx[2]]);
            if (ss.lengthSquared() > 0)
                ss = normalize(ss);
            else
                ss = normalize(isect->dpdu);
        } else
            ss = normalize(isect->dpdu);

         // Compute shading bitangent _ts_ for triangle and adjust _ss_
        Vector3f ts = cross(ss, ns);
        if (ts.lengthSquared() > 0.f) {
            ts = normalize(ts);
            ss = cross(ts, ns);
        } else
            coordinateSystem((Vector3f)ns, &ss, &ts);

         // Compute $\dndu$ and $\dndv$ for triangle shading geometry
        Normal3f dndu, dndv;
        if (_mesh->normals) {
            // Compute deltas for triangle partial derivatives of normal
            Vector2f duv02 = uv[0] - uv[2];
            Vector2f duv12 = uv[1] - uv[2];
            Normal3f dn1 = _mesh->normals[_vertexIdx[0]] - _mesh->normals[_vertexIdx[2]];
            Normal3f dn2 = _mesh->normals[_vertexIdx[1]] - _mesh->normals[_vertexIdx[2]];
            Float determinant = duv02[0] * duv12[1] - duv02[1] * duv12[0];
            bool degenerateUV = std::abs(determinant) < 1e-8;
            if (degenerateUV) {
                 // We can still compute dndu and dndv, with respect to the
                 // same arbitrary coordinate system we use to compute dpdu
                 // and dpdv when this happens. It's important to do this
                 // (rather than giving up) so that ray differentials for
                 // rays reflected from triangles with degenerate
                 // parameterizations are still reasonable.
                Vector3f dn = cross(Vector3f(_mesh->normals[_vertexIdx[2]] - _mesh->normals[_vertexIdx[0]]),
                                     Vector3f(_mesh->normals[_vertexIdx[1]] - _mesh->normals[_vertexIdx[0]]));
                if (dn.lengthSquared() == 0)
                    dndu = dndv = Normal3f(0, 0, 0);
                else {
                    Vector3f dnu, dnv;
                    coordinateSystem(dn, &dnu, &dnv);
                    dndu = Normal3f(dnu);
                    dndv = Normal3f(dnv);
                }
            } else {
                Float invDet = 1 / determinant;
                dndu = (duv12[1] * dn1 - duv02[1] * dn2) * invDet;
                dndv = (-duv12[0] * dn1 + duv02[0] * dn2) * invDet;
            }
        } else
             dndu = dndv = Normal3f(0, 0, 0);
        isect->setShadingGeometry(ss, ts, dndu, dndv, true);
    }

    // Ensure correct orientation of the geometric normal
    if (_mesh->normals)
        isect->normal = faceforward(isect->normal, isect->shading.normal);
    else if (reverseOrientation ^ transformSwapsHandedness)
        isect->normal = isect->shading.normal = -isect->normal;
    *tHit = t;
    return true;
}

/**
 * 基本思路
 * 三角形的参数方程如下
 * u * p0 + v * p1 + (1 - u - v) * p2;
 * 其中p0，p1和p2是三角形的三个点，u, v是p1和p2的权重，1-u-v是p0的权重，并且满足u>=0, v >= 0,u+v<=1
 * ray的方程为o + td
 * 联合以上两个方程得
 * u(p1 - p0) + v(p2 - p0) - td = o - p0
 * E1 = p1 - p0
 * E2 = p2 - p0
 * uE1 + vE2 - td = o - p0
 * T = o - p0
 * 
 * 改写成如下形式
 *                |t|
 * |-d  E1  E2| * |u| = T
 *                |v|
 * 求解上述方程组
 * 把上述向量方程展开成标量方程组之后
 * 根据克拉默法则可得
 *
 *           1
 * t = ------------ |T  E1  E2|
 *     |-d  E1  E2|
 *
 *           1
 * u = ------------ |-d  T  E2|
 *     |-d  E1  E2|
 *
 *           1
 * v = ------------ |-d  E1  T|
 *     |-d  E1  E2|
 *
 * 联合起来，得
 *
 * |t|        1       |T    E1  E2|
 * |u| = ------------ |-d   T   E2|
 * |v|   |-d  E1  E2| |-d   E1   T|
 *
 * 根据混合积公式 |a b c| = a × b · c = - a × c · b
 *
 * |t|         1        |T  × E1 · E2|
 * |u| = -------------- |-d × T  · E2|
 * |v|   |-d × E1 · E2| |-d × E1 ·  T|
 *
 * |t|         1        |T  × E1 · E2|
 * |u| = -------------- | d × E2  · T|
 * |v|    |d × E2 · E1| | T × E1 ·  d|
 *
 * P = d × E2
 * Q = T × E1
 *
 * |t|         1        |Q  · E2|
 * |u| = -------------- |P  ·  T|
 * |v|     |P · E1|     |Q  ·  d|
 * 
 * 求得u>=0, v >= 0,u+v<=1，t <= tMax则有交点
 */
bool Triangle::classicIntersectP(const Ray &ray, bool testAlphaTexture) const {
    const Point3f &p0 = _mesh->points[_vertexIdx[0]];
    const Point3f &p1 = _mesh->points[_vertexIdx[1]];
    const Point3f &p2 = _mesh->points[_vertexIdx[2]];
    
    Float u, v, t;
    
    Vector3f edge1 = p1 - p0;
    Vector3f edge2 = p2 - p0;
    
    Vector3f pvec = cross(ray.dir, edge2);
    
    Float det = dot(edge1, pvec);
    
    if (det == 0) {
        // ray与三角形所在平面平行
        return false;
    }
    Float invDet = 1 / det;
    Vector3f tvec = ray.ori - p0;
    
    u = dot(tvec, pvec) * invDet;
    if (u < 0.0 || u > 1.0) {
        return false;
    }
    
    Vector3f qvec = cross(tvec, edge1);
    
    v = dot(ray.dir, qvec) * invDet;
    
    if (v < 0.0 || u + v > 1.0) {
        return false;
    }
    t = dot(edge2, qvec) * invDet;
    if (t < 0.f || t > ray.tMax) {
        return false;
    }
    
    return true;
}

bool Triangle::classicIntersect(const Ray &ray, Float *tHit,
                                SurfaceInteraction *isect,
                                bool testAlphaTexture) const {
    // todo
    return false;
}

bool Triangle::watertightIntersectP(const Ray &ray, bool testAlphaTexture) const {
    const Point3f &p0 = _mesh->points[_vertexIdx[0]];
    const Point3f &p1 = _mesh->points[_vertexIdx[1]];
    const Point3f &p2 = _mesh->points[_vertexIdx[2]];

    // Perform ray--triangle intersection test

    // Transform triangle vertices to ray coordinate space

    // Translate vertices based on ray origin
    Point3f p0t = p0 - Vector3f(ray.ori);
    Point3f p1t = p1 - Vector3f(ray.ori);
    Point3f p2t = p2 - Vector3f(ray.ori);

    // Permute components of triangle vertices and ray direction
    int kz = maxDimension(abs(ray.dir));
    int kx = kz + 1;
    if (kx == 3) kx = 0;
    int ky = kx + 1;
    if (ky == 3) ky = 0;
    Vector3f d = permute(ray.dir, kx, ky, kz);
    p0t = permute(p0t, kx, ky, kz);
    p1t = permute(p1t, kx, ky, kz);
    p2t = permute(p2t, kx, ky, kz);

    // Apply shear transformation to translated vertex positions
    Float Sx = -d.x / d.z;
    Float Sy = -d.y / d.z;
    Float Sz = 1.f / d.z;
    p0t.x += Sx * p0t.z;
    p0t.y += Sy * p0t.z;
    p1t.x += Sx * p1t.z;
    p1t.y += Sy * p1t.z;
    p2t.x += Sx * p2t.z;
    p2t.y += Sy * p2t.z;

    // Compute edge function coefficients _e0_, _e1_, and _e2_
    Float e0 = p1t.x * p2t.y - p1t.y * p2t.x;
    Float e1 = p2t.x * p0t.y - p2t.y * p0t.x;
    Float e2 = p0t.x * p1t.y - p0t.y * p1t.x;

    // Fall back to double precision test at triangle edges
    if (sizeof(Float) == sizeof(float) &&
        (e0 == 0.0f || e1 == 0.0f || e2 == 0.0f)) {
        double p2txp1ty = (double)p2t.x * (double)p1t.y;
        double p2typ1tx = (double)p2t.y * (double)p1t.x;
        e0 = (float)(p2typ1tx - p2txp1ty);
        double p0txp2ty = (double)p0t.x * (double)p2t.y;
        double p0typ2tx = (double)p0t.y * (double)p2t.x;
        e1 = (float)(p0typ2tx - p0txp2ty);
        double p1txp0ty = (double)p1t.x * (double)p0t.y;
        double p1typ0tx = (double)p1t.y * (double)p0t.x;
        e2 = (float)(p1typ0tx - p1txp0ty);
    }

    // Perform triangle edge and determinant tests
    if ((e0 < 0 || e1 < 0 || e2 < 0) && (e0 > 0 || e1 > 0 || e2 > 0))
        return false;
    Float det = e0 + e1 + e2;
    if (det == 0)
        return false;

    // Compute scaled hit distance to triangle and test against ray $t$ range
    p0t.z *= Sz;
    p1t.z *= Sz;
    p2t.z *= Sz;
    Float tScaled = e0 * p0t.z + e1 * p1t.z + e2 * p2t.z;
    if (det < 0 && (tScaled >= 0 || tScaled < ray.tMax * det))
        return false;
    else if (det > 0 && (tScaled <= 0 || tScaled > ray.tMax * det))
        return false;

    // Compute barycentric coordinates and $t$ value for triangle intersection
    Float invDet = 1 / det;
    Float b0 = e0 * invDet;
    Float b1 = e1 * invDet;
    Float b2 = e2 * invDet;
    Float t = tScaled * invDet;

    // Ensure that computed triangle $t$ is conservatively greater than zero

    // Compute $\delta_z$ term for triangle $t$ error bounds
    Float maxZt = maxComponent(abs(Vector3f(p0t.z, p1t.z, p2t.z)));
    Float deltaZ = gamma(3) * maxZt;

    // Compute $\delta_x$ and $\delta_y$ terms for triangle $t$ error bounds
    Float maxXt = maxComponent(abs(Vector3f(p0t.x, p1t.x, p2t.x)));
    Float maxYt = maxComponent(abs(Vector3f(p0t.y, p1t.y, p2t.y)));
    Float deltaX = gamma(5) * (maxXt + maxZt);
    Float deltaY = gamma(5) * (maxYt + maxZt);

    // Compute $\delta_e$ term for triangle $t$ error bounds
    Float deltaE =
        2 * (gamma(2) * maxXt * maxYt + deltaY * maxXt + deltaX * maxYt);

    // Compute $\delta_t$ term for triangle $t$ error bounds and check _t_
    Float maxE = maxComponent(abs(Vector3f(e0, e1, e2)));
    Float deltaT = 3 *
                   (gamma(3) * maxE * maxZt + deltaE * maxZt + deltaZ * maxE) *
                   std::abs(invDet);
    if (t <= deltaT)
        return false;

    // Test shadow ray intersection against alpha texture, if present
    if (testAlphaTexture && (_mesh->alphaMask || _mesh->shadowAlphaMask)) {
        // Compute triangle partial derivatives
        Vector3f dpdu, dpdv;
        Point2f uv[3];
        getUVs(uv);

        // Compute deltas for triangle partial derivatives
        Vector2f duv02 = uv[0] - uv[2], duv12 = uv[1] - uv[2];
        Vector3f dp02 = p0 - p2, dp12 = p1 - p2;
        Float determinant = duv02[0] * duv12[1] - duv02[1] * duv12[0];
        bool degenerateUV = std::abs(determinant) < 1e-8;
        if (!degenerateUV) {
            Float invdet = 1 / determinant;
            dpdu = (duv12[1] * dp02 - duv02[1] * dp12) * invdet;
            dpdv = (-duv12[0] * dp02 + duv02[0] * dp12) * invdet;
        }
        if (degenerateUV || cross(dpdu, dpdv).lengthSquared() == 0) {
            // Handle zero determinant for triangle partial derivative matrix
            Vector3f ng = cross(p2 - p0, p1 - p0);
            if (ng.lengthSquared() == 0)
                // The triangle is actually degenerate; the intersection is
                // bogus.
                return false;

            coordinateSystem(normalize(cross(p2 - p0, p1 - p0)), &dpdu, &dpdv);
        }

        // Interpolate $(u,v)$ parametric coordinates and hit point
        Point3f pHit = b0 * p0 + b1 * p1 + b2 * p2;
        Point2f uvHit = b0 * uv[0] + b1 * uv[1] + b2 * uv[2];
        SurfaceInteraction isectLocal(pHit, Vector3f(0, 0, 0), uvHit, -ray.dir,
                                      dpdu, dpdv, Normal3f(0, 0, 0),
                                      Normal3f(0, 0, 0), ray.time, this);
        if (_mesh->alphaMask && _mesh->alphaMask->evaluate(isectLocal) == 0)
            return false;
        if (_mesh->shadowAlphaMask &&
            _mesh->shadowAlphaMask->evaluate(isectLocal) == 0)
            return false;
    }
    return true;
}

bool Triangle::intersectP(const Ray &ray, bool testAlphaTexture ) const {
    return watertightIntersectP(ray, testAlphaTexture);
}

bool Triangle::intersect(const Ray &ray, Float *tHit, SurfaceInteraction *isect, bool testAlphaTexture) const {
    return watertightIntersect(ray, tHit, isect, testAlphaTexture);
}

AABB3f Triangle::worldBound() const {
    const Point3f &p0 = _mesh->points[_vertexIdx[0]];
    const Point3f &p1 = _mesh->points[_vertexIdx[1]];
    const Point3f &p2 = _mesh->points[_vertexIdx[2]];
    AABB3f b1 = AABB3f(p0, p1);
    return unionSet(b1, p2);
}

Interaction Triangle::samplePos(const Point2f &u, Float *pdf) const {
    Interaction ret;
    Point2f b = uniformSampleTriangle(u);
    const Point3f &p0 = _mesh->points[_vertexIdx[0]];
    const Point3f &p1 = _mesh->points[_vertexIdx[1]];
    const Point3f &p2 = _mesh->points[_vertexIdx[2]];
    // 2D三角形坐标转换为3D空间三角形坐标
    ret.pos = b[0] * p0 + b[1] * p1 + (1 - b[0] - b[1]) * p2;
    
    ret.normal = normalize(Normal3f(cross(p1 - p0, p2 - p0)));
    
    if (_mesh->normals) {
        Normal3f ns(b[0] * _mesh->normals[_vertexIdx[0]] + b[1] * _mesh->normals[_vertexIdx[1]] +
                    (1 - b[0] - b[1]) * _mesh->normals[_vertexIdx[2]]);
        ret.normal = faceforward(ret.normal, ns);
    } else if (reverseOrientation ^ transformSwapsHandedness) {
        ret.normal *= -1;
    }
    Point3f pAbsSum = abs(b[0] * p0) + abs(b[1] * p1) + abs((1 - b[0] - b[1]) * p2);
    //todo 推导过程后续补上
    ret.pError = gamma(6) * Vector3f(pAbsSum.x, pAbsSum.y, pAbsSum.z);
    *pdf = pdfPos();
    return ret;
}

shared_ptr<TriangleMesh> createTriMesh(const shared_ptr<const Transform> &o2w, int nTriangles,
                                    const int *vertexIndices, int nVertices, const Point3f *P,
                                    const Point2f *uv, const Normal3f *N, const Vector3f *S,
                                    const std::shared_ptr<Texture<Float>> &alphaMask,
                                    const std::shared_ptr<Texture<Float>> &shadowAlphaMask,
                                    const int *faceIndices) {
    return make_shared<TriangleMesh>(o2w, nTriangles,
                                    vertexIndices,
                                    nVertices,
                                    P, S, N, uv,
                                    alphaMask,
                                    shadowAlphaMask,
                                    faceIndices);
}

shared_ptr<Triangle> createTri(const shared_ptr<const Transform> &o2w, shared_ptr<const Transform> w2o,
                                bool reverseOrientation,
                                const std::shared_ptr<TriangleMesh> &_mesh,
                                int triNumber) {
    return make_shared<Triangle>(o2w, w2o, reverseOrientation, _mesh, triNumber);
}

vector<shared_ptr<Shape>> createQuad(const shared_ptr<const Transform> &o2w,
                                bool reverseOrientation,
                                Float width, Float height,
                                const MediumInterface &mediumInterface) {
    if (height == 0) {
        height = width;
    }
    width /= 2.f;
    height /= 2.f;
    DCHECK(width > 0);
    DCHECK(height > 0);
    Point3f tr(width, height, 0);
    Point3f tl(-width, height, 0);
    Point3f br(width, -height, 0);
    Point3f bl(-width, -height,0);
    
    Point3f points[] = {tl, bl, br, tr};
    int vertIndice[6] = {0,1,2, 0,2,3};
    Point2f UVs[] = {Point2f(0, 1), Point2f(0, 0), Point2f(1, 0), Point2f(1,1)};
    int nTri = 2;
    int nVert = 4;
    auto mesh = createTriMesh(o2w, nTri,vertIndice, nVert, points, UVs);
    vector<shared_ptr<Shape>> ret;
    shared_ptr<Transform>w2o(o2w->getInverse_ptr());
    for (int i = 0; i < nTri; ++i) {
        ret.push_back(createTri(o2w, w2o, reverseOrientation, mesh, i));
    }
    return ret;
}

//"param" : {
//    "transform" : {
//        "type" : "translate",
//        "param" : [0,0,0]
//    },
//    "width" : 1,
//    "height" : 1,
//    "reverseOrientation" : false
//}
vector<shared_ptr<Primitive>> createQuadPrimitive(const nloJson &data,
                                                  const shared_ptr<const Material>& mat,
                                                  vector<shared_ptr<Light>> &lights,
                                                  const MediumInterface &mediumInterface) {
    nloJson param = data.value("param", nloJson::object());
    auto l2w = createTransform(param.value("transform", nloJson()));
    bool ro = param.value("reverseOrientation", false);
    shared_ptr<Transform> o2w(l2w);
    Float width = param.value("width", 1.f);
    Float height = param.value("height", width);
    vector<shared_ptr<Shape>> triLst = createQuad(o2w, ro, width, height);
    nloJson emissionData = data.value("emission", nloJson());
    return createPrimitive(triLst, lights, mat, mediumInterface, emissionData);
}

vector<shared_ptr<Shape>> createCube(const shared_ptr<const Transform> &o2w,
                                     bool reverseOrientation,
                                     Float x, Float y, Float z,
                                     const MediumInterface &mediumInterface) {
    vector<shared_ptr<Shape>> ret;
    Float hx = 0.5 * x;
    Float hy = 0.5 * y;
    Float hz = 0.5 * z;
    auto znTr(Transform::translate_ptr(0,0,-hz));
    * znTr = (*znTr) * Transform::rotateX(180);
    * znTr = *o2w * (*znTr);
    auto znTris = createQuad(shared_ptr<const Transform>(znTr), reverseOrientation, x,y,mediumInterface);
    ret.insert(ret.end(), znTris.begin(), znTris.end());

    auto zpTr(Transform::translate_ptr(0,0,hz));
    * zpTr = *o2w * (*zpTr);
    auto zpTris = createQuad(shared_ptr<const Transform>(zpTr), reverseOrientation, x,y,mediumInterface);
    ret.insert(ret.end(), zpTris.begin(), zpTris.end());

    auto ynTr(Transform::translate_ptr(0, -hy, 0));
    * ynTr = (*ynTr) * Transform::rotateX(90);
    * ynTr = *o2w * (*ynTr);
    auto ynTris = createQuad(shared_ptr<const Transform>(ynTr), reverseOrientation, x,z,mediumInterface);
    ret.insert(ret.end(), ynTris.begin(), ynTris.end());

    auto ypTr(Transform::translate_ptr(0, hy, 0));
    * ypTr = (*ypTr) * Transform::rotateX(-90);
    * ypTr = *o2w * (*ypTr);
    auto ypTris = createQuad(shared_ptr<const Transform>(ypTr), reverseOrientation, x,z,mediumInterface);
    ret.insert(ret.end(), ypTris.begin(), ypTris.end());

    auto xnTr(Transform::translate_ptr(-hx, 0, 0));
    * xnTr = (*xnTr) * Transform::rotateY(-90);
    * xnTr = *o2w * (*xnTr);
    auto xnTris = createQuad(shared_ptr<const Transform>(xnTr), reverseOrientation, z,y,mediumInterface);
    ret.insert(ret.end(), xnTris.begin(), xnTris.end());

    auto xpTr(Transform::translate_ptr(hx, 0, 0));
    * xpTr = (*xpTr) * Transform::rotateY(90);
    * xpTr = *o2w * (*xpTr);
    auto xpTris = createQuad(shared_ptr<const Transform>(xpTr), reverseOrientation, z,y);
    ret.insert(ret.end(), xpTris.begin(), xpTris.end());
    
    
    return ret;
}

//data : {
//    "type" : "triMesh",
//    "subType" : "cube",
//    "name" : "cube1",
//    "enable" : true,
//    "param" : {
//        "transform" :[
//            {
//                "type" : "rotateY",
//                "param" : [-20]
//            },
//            {
//                "type" : "translate",
//                "param" : [-0.3,-0.4,0.2]
//            }
//        ],
//        "x" : 0.6,
//        "y" : 1.2,
//        "z" : 0.6
//    },
//    "material" : "matte1"
//}
vector<shared_ptr<Primitive>> createCubePrimitive(const nloJson &data,
                                                  const shared_ptr<const Material>&mat,
                                                  vector<shared_ptr<Light>> &lights,
                                                  const MediumInterface &mediumInterface) {
    nloJson param = data.value("param", nloJson::object());
    auto l2w = createTransform(param.value("transform", nloJson()));
    bool ro = param.value("reverseOrientation", false);
    shared_ptr<Transform> o2w(l2w);
    Float x = param.value("x", 1.f);
    Float y = param.value("y", x);
    Float z = param.value("z", y);
    vector<shared_ptr<Shape>> triLst = createCube(o2w, ro, x, y, z, mediumInterface);
    nloJson emission = data.value("emission", nloJson());
    return createPrimitive(triLst, lights, mat, mediumInterface, emission);
}

//data : {
//    "type" : "triMesh",
//    "subType" : "model",
//    "name" : "box",
//    "enable" : true,
//    "param" : {
//        "transform" :[
//            {
//                "type" : "rotateY",
//                "param" : [15]
//            },
//            {
//                "type" : "translate",
//                "param" : [0.35,-0.7,-0.4]
//            }
//        ],
//        "fileName" : "res/box.obj"
//    },
//    "material" : "matte1"
//}
vector<shared_ptr<Primitive>> createModelPrimitive(const nloJson &data,
                                                   const shared_ptr<const Material> &mat,
                                                   vector<shared_ptr<Light>> &lights,
                                                   const MediumInterface &mediumInterface) {
    nloJson param = data.value("param", nloJson::object());
    string fileName = param.value("fileName", "");
    string basePath = param.value("basePath", "");
    auto l2w = shared_ptr<const Transform>(createTransform(param.value("transform", nloJson())));
    bool ro = param.value("reverseOrientation", false);
    nloJson emissionData = data.value("emission", nloJson());
    vector<shared_ptr<Shape>> triLst = createTriFromFile(fileName, l2w, ro, basePath);
    return createPrimitive(triLst, lights, mat, mediumInterface, emissionData);
}

vector<shared_ptr<Shape>> createTriFromFile(const string &fn,
                                            const shared_ptr<const Transform> &o2w,
                                            bool reverseOrientation,
                                            const string &basePath) {
    vector<shared_ptr<Shape>> ret;
    ModelParser mp;
    mp.load(fn, basePath);
    return mp.getTriLst(o2w, reverseOrientation);
}

vector<shared_ptr<Primitive>> createPrimitive(const vector<shared_ptr<Shape>> &triLst,
                                              vector<shared_ptr<Light>> &lights,
                                              const shared_ptr<const Material> &mat,
                                              const MediumInterface &mediumInterface,
                                              const nloJson &emissionData) {
    vector<shared_ptr<Primitive>> ret;
    for (int i = 0; i < triLst.size(); ++i) {
        auto shape = triLst.at(i);
        shared_ptr<DiffuseAreaLight> areaLight(createDiffuseAreaLight(emissionData, shape));
        if (areaLight) {
            lights.push_back(areaLight);
        }
        shared_ptr<Primitive> primitives = GeometricPrimitive::create(shape, mat, areaLight, mediumInterface);
        ret.push_back(primitives);
    }
    return ret;
}

PALADIN_END
