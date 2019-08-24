//
//  perspective.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "perspective.hpp"
#include "sampling.hpp"

PALADIN_BEGIN

PerspectiveCamera::PerspectiveCamera(const AnimatedTransform &CameraToWorld,
                                     const AABB2f &screenWindow,
                                     Float shutterOpen, Float shutterClose,
                                     Float lensRadius, Float focalDistance,
                                     Float fov, Film *film,
                                     const Medium *medium)
: ProjectiveCamera(CameraToWorld, Transform::perspective(fov, 1e-2f, 1000.f),
screenWindow, shutterOpen, shutterClose, lensRadius,
                   focalDistance, film, medium) {
   
    dxCamera = (_rasterToCamera.exec(Point3f(1, 0, 0)) - _rasterToCamera.exec(Point3f(0, 0, 0)));
    dyCamera = (_rasterToCamera.exec(Point3f(0, 1, 0)) - _rasterToCamera.exec(Point3f(0, 0, 0)));
    
    
    Point2i res = film->fullResolution;
    Point3f pMin = _rasterToCamera.exec(Point3f(0, 0, 0));
    Point3f pMax = _rasterToCamera.exec(Point3f(res.x, res.y, 0));
    pMin /= pMin.z;
    pMax /= pMax.z;
    A = std::abs((pMax.x - pMin.x) * (pMax.y - pMin.y));
}

Float PerspectiveCamera::generateRay(const CameraSample &sample,
                                     Ray *ray) const {
    // 在胶片上采样的点
    Point3f pFilm = Point3f(sample.pFilm.x, sample.pFilm.y, 0);
    Point3f pCamera = _rasterToCamera.exec(pFilm);
    *ray = Ray(Point3f(0, 0, 0), normalize(Vector3f(pCamera)));
    if (_lensRadius > 0) {

        Point2f pLens = _lensRadius * (sample.pLens);
        
        Float ft = _focalDistance / ray->dir.z;
        Point3f pFocus = ray->at(ft);
        
        ray->ori = Point3f(pLens.x, pLens.y, 0);
        ray->dir = normalize(pFocus - ray->ori);
    }
    ray->time = lerp(sample.time, shutterOpen, shutterClose);
    ray->medium = medium;
    *ray = cameraToWorld.exec(*ray);
    return 1;
}

PALADIN_END
