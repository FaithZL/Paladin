//
//  camera.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef camera_hpp
#define camera_hpp

#include "header.h"
#include "film.hpp"
#include "animatedtransform.hpp"

PALADIN_BEGIN


class Camera {
public:
    Camera(const AnimatedTransform &CameraToWorld, Float shutterOpen,
           Float shutterClose, Film *film, const Medium *medium);
    
    virtual ~Camera();
    
    virtual Float generateRay(const CameraSample &sample, Ray *ray) const = 0;
    
    virtual Float generateRayDifferential(const CameraSample &sample,
                                          RayDifferential *rd) const;
    
    virtual Spectrum we(const Ray &ray, Point2f *pRaster2 = nullptr) const;
    
    virtual void pdfWe(const Ray &ray, Float *pdfPos, Float *pdfDir) const;
    
    virtual Spectrum sampleWi(const Interaction &ref, const Point2f &u,
                               Vector3f *wi, Float *pdf, Point2f *pRaster,
                               VisibilityTester *vis) const;
    

    AnimatedTransform cameraToWorld;
    // 快门开启时间，快门关闭时间
    const Float shutterOpen, shutterClose;
    Film * film;
    // 相机所在的介质
    const Medium *medium;
};

struct CameraSample {
    Point2f pFilm;
    Point2f pLens;
    Float time;
};

class ProjectiveCamera : public Camera {
public:

    ProjectiveCamera(const AnimatedTransform &CameraToWorld,
                     const Transform &cameraToScreen,
                     const AABB2f &screenWindow, Float shutterOpen,
                     Float shutterClose, Float lensr, Float focald, Film *film,
                     const Medium *medium)
    : Camera(CameraToWorld, shutterOpen, shutterClose, film, medium),
    _cameraToScreen(cameraToScreen) {

        _lensRadius = lensr;
        _focalDistance = focald;
        
        _screenToRaster = Transform::scale(film->fullResolution.x, film->fullResolution.y, 1) *
        Transform::scale(1 / (screenWindow.pMax.x - screenWindow.pMin.x),
              1 / (screenWindow.pMin.y - screenWindow.pMax.y), 1) *
        Transform::translate(Vector3f(-screenWindow.pMin.x, -screenWindow.pMax.y, 0));
        _rasterToScreen = _screenToRaster.getInverse();
        _rasterToCamera = _cameraToScreen.getInverse() * _rasterToScreen;
    }
    
protected:

    Transform _cameraToScreen, _rasterToCamera;
    Transform _screenToRaster, _rasterToScreen;
    
    // 透镜半径
    Float _lensRadius;
    // 焦距
    Float _focalDistance;
};

PALADIN_END

#endif /* camera_hpp */
