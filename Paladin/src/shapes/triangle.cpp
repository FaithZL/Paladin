//
//  triangle.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/5/3.
//

#include "triangle.hpp"
#include "math/sampling.hpp"
#include "mesh.hpp"


PALADIN_BEGIN

STAT_PERCENT("Intersections/Ray-triangle intersection tests", nHits, nTests);

Point3f TriangleI::sample(const Point3f *positions, const Normal3f *normals,
                          const Point2f *texCoords, Normal3f * normal, Point2f * uv,
                          const Point2f &u) const {
    const Point3f &p0 = positions[indice[0].pos];
    const Point3f &p1 = positions[indice[1].pos];
    const Point3f &p2 = positions[indice[2].pos];
    
    Point2f bary = uniformSampleTriangle(u);
    
    Vector3f sideA = p1 - p0, sideB = p2 - p0;
    Point3f p = p0 + (sideA * bary.x) + (sideB * bary.y);
    
    if (normals) {
        const Normal3f &n0 = normals[indice[0].normal];
        const Normal3f &n1 = normals[indice[1].normal];
        const Normal3f &n2 = normals[indice[2].normal];

        * normal = Normal3f(normalize(
            n0 * (1.0f - bary.x - bary.y) +
            n1 * bary.x + n2 * bary.y
        ));
    } else {
        * normal = Normal3f(normalize(cross(sideA, sideB)));
    }
    
    if (texCoords) {
        const Point2f &uv0 = texCoords[indice[0].uv];
        const Point2f &uv1 = texCoords[indice[1].uv];
        const Point2f &uv2 = texCoords[indice[2].uv];

        * uv = uv0 * (1.0f - bary.x - bary.y) +
            uv1 * bary.x + uv2 * bary.y;
    } else {
        * uv = bary;
    }
    
    return p;
}

bool TriangleI::rayOccluded(const Ray &ray, bool testAlphaTexture) const {
    Float u,v,t;
    bool ret = rayIntersect(parent->_points.get(), ray, &u, &v, &t);
    if (ret) {
        ray.tMax = t;
    }
    return ret;
}

AABB3f TriangleI::worldBound() const {
    auto ret = worldBound(parent->_points.get());
    return ret;
}

void TriangleI::getUVs(Point2f uv[3]) const {
    if (parent->_uv) {
        uv[0] = indice[0].uv < 0 ? Point2f(0, 0) : parent->_uv[indice[0].uv];
        uv[1] = indice[1].uv < 0 ? Point2f(1, 0) : parent->_uv[indice[1].uv];
        uv[2] = indice[2].uv < 0 ? Point2f(1, 1) : parent->_uv[indice[2].uv];
    } else {
        uv[0] = Point2f(0, 0);
        uv[1] = Point2f(1, 0);
        uv[2] = Point2f(1, 1);
    }
}

bool TriangleI::fillSurfaceInteraction(const Ray &ray, const Vector2f &uv, SurfaceInteraction *isect) const {
    TRY_PROFILE(Prof::triFillSurfaceInteraction)
    Float b1 = uv.x;
    Float b2 = uv.y;
    Float b0 = 1.f - b1 - b2;
    
    const Point3f &p0 = parent->_points[indice[0].pos];
    const Point3f &p1 = parent->_points[indice[1].pos];
    const Point3f &p2 = parent->_points[indice[2].pos];
    
    Vector3f dp02 = p0 - p2;
    Vector3f dp12 = p1 - p2;
    
    Point3f pHit = b0 * p0 + b1 * p1 + b2 * p2;
    
    Point2f uv2[3];
    getUVs(uv2);
    
    Point2f uvHit = b0 * uv2[0] + b1 * uv2[1] + b2 * uv2[2];
    Normal3f normal(normalize(cross(dp02, dp12)));
    
    Vector2f duv02 = uv2[0] - uv2[2];
    Vector2f duv12 = uv2[1] - uv2[2];
    
    Float det = duv02[0] * duv12[1] - duv02[1] * duv12[0];
    bool degenerateUV = std::abs(det) < 1e-8;
    
    Vector3f dpdu;
    Vector3f dpdv;

    Float invDet = 1 / det;
    if (!degenerateUV) {
        dpdu = (duv12[1] * dp02 - duv02[1] * dp12) * invDet;
        dpdv = (-duv12[0] * dp02 + duv02[0] * dp12) * invDet;
    }
    
    if (degenerateUV || cross(dpdu, dpdv).lengthSquared() == 0) {
        Vector3f ng = cross(p2 - p0, p1 - p0);
        if (ng.lengthSquared() == 0) {
            return false;
        }
        coordinateSystem(normalize(ng), &dpdu, &dpdv);
    }
    isect->normal = isect->shading.normal = Normal3f(normalize(cross(dp02, dp12)));
    
    
    Vector3f pError = Vector3f();
    
    *isect = SurfaceInteraction(pHit, pError, uvHit, -ray.dir, dpdu, dpdv,
                                Normal3f(0, 0, 0), Normal3f(0, 0, 0), ray.time,
                                parent, 0);
    
    isect->normal = isect->shading.normal = Normal3f(normalize(cross(dp02, dp12)));
    if (parent->_normals) {
        Normal3f ns;
        if (parent->_normals) {
            ns = (b0 * parent->_normals[indice[0].normal] + b1 * parent->_normals[indice[1].normal] + b2 * parent->_normals[indice[2].normal]);
            if (ns.lengthSquared() > 0) {
                ns = normalize(ns);
            } else {
                ns = isect->normal;
            }
        } else {
             ns = isect->normal;
        }
        
        Vector3f ss = normalize(isect->dpdu);
        Vector3f ts = cross(ss, ns);
        
        if (ts.lengthSquared() > 0.f) {
            ts = normalize(ts);
            ss = cross(ts, ns);
        } else {
            coordinateSystem((Vector3f)ns, &ss, &ts);
        }
        
        Normal3f dndu, dndv;
        if (parent->_normals) {
            Vector2f duv02 = uv2[0] - uv2[2];
            Vector2f duv12 = uv2[1] - uv2[2];
            Normal3f dn1 = parent->_normals[indice[0].normal] - parent->_normals[indice[2].normal];
            Normal3f dn2 = parent->_normals[indice[1].normal] - parent->_normals[indice[2].normal];
            Float determinant = duv02[0] * duv12[1] - duv02[1] * duv12[0];
            bool degenerateUV = std::abs(determinant) < 1e-8;
            if (degenerateUV) {

                Vector3f dn = cross(Vector3f(parent->_normals[indice[2].normal] - parent->_normals[indice[0].normal]),
                                     Vector3f(parent->_normals[indice[1].normal] - parent->_normals[indice[0].normal]));
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
        } else {
            dndu = dndv = Normal3f(0, 0, 0);
        }
        isect->setShadingGeometry(ss, ts, dndu, dndv, true);
    }
    
    if (parent->_mediumInterface.isMediumTransition()){
        isect->mediumInterface = parent->_mediumInterface;
    } else {
        isect->mediumInterface = MediumInterface(ray.medium);
    }
    
    if (parent->_normals)
        isect->normal = faceforward(isect->normal, isect->shading.normal);
    else if (parent->reverseOrientation ^ parent->transformSwapsHandedness)
        isect->normal = isect->shading.normal = -isect->normal;
    return true;
}

bool TriangleI::rayIntersect(const Ray &ray,
                            SurfaceInteraction * isect,
                            bool testAlphaTexture) const {
    ++nTests;
    Float u,v,t;
    bool ret = rayIntersect(parent->_points.get(), ray, &u, &v, &t);
    if (ret) {
        ++nHits;
        ray.tMax = t;
        fillSurfaceInteraction(ray, Vector2f(u, v), isect);
    }
    return ret;
}

Point3f TriangleI::sample(const Point3f *positions, const Point2f &u) const {
    const Point3f &p0 = positions[indice[0].pos];
    const Point3f &p1 = positions[indice[1].pos];
    const Point3f &p2 = positions[indice[2].pos];
    Point2f bary = uniformSampleTriangle(u);
    Vector3f sideA = p1 - p0, sideB = p2 - p0;
    Point3f p = p0 + (sideA * bary.x) + (sideB * bary.y);
    return p;
}

Interaction TriangleI::sample(const Point2f &u) const {
    Interaction ret;
    Point2f uv;
    ret.pos = sample(parent->_points.get(),
                     parent->_normals.get(),
                     parent->_uv.get(),
                     &ret.normal, &uv, u);
    return ret;
}

PALADIN_END
