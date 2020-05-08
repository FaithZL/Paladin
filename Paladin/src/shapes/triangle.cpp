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

bool TriangleI::rayOccluded(const Ray &ray, bool testAlphaTexture) {
    Float u,v,t;
    return rayIntersect(parent->_points.get(), ray, &u, &v, &t);
}

AABB3f TriangleI::worldBound() const {
    return worldBound(parent->_points.get());
}

bool TriangleI::rayIntersect(const Ray &ray,
                            Float *tHit,
                            SurfaceInteraction * isect,
                            bool testAlphaTexture) {
    
    
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

PALADIN_END
