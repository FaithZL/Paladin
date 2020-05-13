//
//  triangle.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/5/3.
//

#ifndef triangle_hpp
#define triangle_hpp

#include "core/header.h"
#include "core/shape.hpp"
#include "core/texture.hpp"
#include <vector>
#include "math/frame.hpp"

PALADIN_BEGIN

struct IndexSet {
    IndexSet(int pos, int uv, int normal):
    uv(uv),
    pos(pos),
    normal(normal) {
        
    }
    
    IndexSet(int idx)
    : uv(idx),
    pos(idx),
    normal(idx) {
        
    }
    
    IndexSet() {
        uv = 0;
        pos = 0;
        normal = 0;
    }
    
    // 顶点uv索引
    int uv;
    // 顶点位置索引
    int pos;
    // 顶点法线索引
    int normal;
};

class Mesh;

struct TriangleI : EmbreeUtil::EmbreeGeomtry {
    
    TriangleI(const IndexSet * p, const Mesh * mesh = nullptr)
    : indice(p),
    parent(mesh) {
        
    }
    
    virtual AABB3f worldBound() const;
    
    F_INLINE AABB3f worldBound(const Point3f *positions) const {
        const Point3f &p0 = positions[indice[0].pos];
        const Point3f &p1 = positions[indice[1].pos];
        const Point3f &p2 = positions[indice[2].pos];
        AABB3f ret(p0, p1);
        ret = unionSet(ret, p2);
        return ret;
    }
    
    F_INLINE Float getArea(const Point3f *positions) const {
        const Point3f &p0 = positions[indice[0].pos];
        const Point3f &p1 = positions[indice[1].pos];
        const Point3f &p2 = positions[indice[2].pos];
        Vector3f sideA = p1 - p0, sideB = p2 - p0;
        return 0.5f * cross(sideA, sideB).length();
    }
    
    F_INLINE static bool rayIntersect(const Point3f &p0, const Point3f &p1, const Point3f &p2,
                                     const Ray &ray, Float *u, Float *v, Float *t) {

        Vector3f edge1 = p1 - p0, edge2 = p2 - p0;

        Vector3f pvec = cross(ray.dir, edge2);

        Float det = dot(edge1, pvec);
        if (det == 0) {
            return false;
        }
        Float inv_det = 1.0f / det;

        Vector3f tvec = ray.ori - p0;

        *u = dot(tvec, pvec) * inv_det;
        if (*u < 0.0 || *u > 1.0) {
            return false;
        }

        Vector3f qvec = cross(tvec, edge1);

        *v = dot(ray.dir, qvec) * inv_det;

        if (*v >= 0.0 && *u + *v <= 1.0) {
            *t = dot(edge2, qvec) * inv_det;
            if (*t <= ray.tMax) {
                ray.tMax = *t;
                return true;
            }
            return false;
        }
        return false;
    }
    
    virtual bool fillSurfaceInteraction(const Ray &r, const Vector2f &uv, SurfaceInteraction *isect) const;
    
    F_INLINE bool rayIntersect(const Point3f *positions, const Ray &ray, Float *u,
        Float *v, Float *t) const {
        return rayIntersect(
            positions[indice[0].pos], positions[indice[1].pos],
            positions[indice[2].pos], ray, u, v, t);
    }
    
    virtual bool rayIntersect(const Ray &ray,
                            SurfaceInteraction * isect,
                            bool testAlphaTexture = true) const;
    
    virtual bool rayOccluded(const Ray &ray, bool testAlphaTexture = true) const;
    
    Point3f sample(const Point3f *positions, const Normal3f *normals,
                    const Point2f *texCoords, Normal3f * normal, Point2f * uv,
                    const Point2f &u) const;
    
    Point3f sample(const Point3f *positions, const Point2f &u) const;

    F_INLINE void getUVs(Point2f uv[3]) const;
    
//private:
    const Mesh * parent;
    const IndexSet * indice;
};

PALADIN_END

#endif /* triangle_hpp */
