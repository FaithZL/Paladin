//
//  perspective.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "cameras/perspective.hpp"
#include "math/sampling.hpp"

PALADIN_BEGIN

PerspectiveCamera::PerspectiveCamera(const AnimatedTransform &CameraToWorld,
                                     const AABB2f &screenWindow,
                                     Float shutterOpen, Float shutterClose,
                                     Float lensRadius, Float focalDistance,
                                     Float fov, shared_ptr<Film> film,
                                     const Medium *medium)
: ProjectiveCamera(CameraToWorld, Transform::perspective(fov, 1e-2f, 1000.f),
screenWindow, shutterOpen, shutterClose, lensRadius,
                   focalDistance, film, medium) {
    // 通过光栅空间的微分获取到相机空间的微分
    _dxCamera = (_rasterToCamera.exec(Point3f(1, 0, 0)) - _rasterToCamera.exec(Point3f(0, 0, 0)));
    _dyCamera = (_rasterToCamera.exec(Point3f(0, 1, 0)) - _rasterToCamera.exec(Point3f(0, 0, 0)));
    
    // 计算相机空间下，z=1的情况下的film的面积
    Point2i res = film->fullResolution;
    Point3f pMin = _rasterToCamera.exec(Point3f(0, 0, 0));
    Point3f pMax = _rasterToCamera.exec(Point3f(res.x, res.y, 0));
    pMin /= pMin.z;
    pMax /= pMax.z;
    _area = std::abs((pMax.x - pMin.x) * (pMax.y - pMin.y));
}

Float PerspectiveCamera::generateRay(const CameraSample &sample, Ray *ray) const {
    // 在胶片上采样的点
    Point3f pFilm = Point3f(sample.pFilm.x, sample.pFilm.y, 0);
    // 把光栅空间转到相机空间
    Point3f pCamera = _rasterToCamera.exec(pFilm);
    // 生成光线
    *ray = Ray(Point3f(0, 0, 0), normalize(Vector3f(pCamera)));
    
    // 如果透镜半径大于0，则需要重新确定光线的起点以及方向
    if (_lensRadius > 0) {
        Point2f pLens = _lensRadius * uniformSampleDisk(sample.pLens);
        
        /**
         * 聚焦平面上的样本点，发出的所有光线通过透镜之后都会聚焦在film的一个点上，
         * 但如果光线通过透镜中心，方向不会改变，我们可以以此来确定聚焦平面上的样本点pFocus
         * 可以通过相似三角形来推导出以下表达式
         */
        Float ft = _focalDistance / ray->dir.z;
        // 计算聚焦平面上的点
        Point3f pFocus = ray->at(ft);
        
        // 由于透镜的影响，更新ray属性
        // 聚焦平面上的点与透镜上的样本点确定新的光线
        ray->ori = Point3f(pLens.x, pLens.y, 0);
        ray->dir = normalize(pFocus - ray->ori);
    }
    ray->time = lerp(sample.time, shutterOpen, shutterClose);
    ray->medium = medium;
    *ray = cameraToWorld.exec(*ray);
    return 1;
}

Float PerspectiveCamera::generateRayDifferential(const CameraSample &sample, RayDifferential *ray) const {
    Point3f pFilm = Point3f(sample.pFilm.x, sample.pFilm.y, 0);
    Point3f pCamera = _rasterToCamera.exec(pFilm);
    
    Vector3f dir = normalize(Vector3f(pCamera));
    *ray = RayDifferential(Point3f(0, 0, 0), dir);
    
    if (_lensRadius > 0) {
        Point2f pLens = _lensRadius * uniformSampleDisk(sample.pLens);
        
        // 计算聚焦平面
        Float ft = _focalDistance / ray->dir.z;
        Point3f pFocus = ray->at(ft);
        // 由于透镜的影响，更新ray属性
        ray->ori = Point3f(pLens.x, pLens.y, 0);
        ray->dir = normalize(pFocus - ray->ori);
    }
    
    // 计算光线微分，计算方法跟generateRay函数中的类似，因此不再解释
    if (_lensRadius > 0) {
        Point2f pLens = _lensRadius * uniformSampleDisk(sample.pLens);
        Vector3f dx = normalize(Vector3f(pCamera + _dxCamera));
        Float ft = _focalDistance / dx.z;
        Point3f pFocus = Point3f(0, 0, 0) + (ft * dx);
        ray->rxOrigin = Point3f(pLens.x, pLens.y, 0);
        ray->rxDirection = normalize(pFocus - ray->rxOrigin);
        
        Vector3f dy = normalize(Vector3f(pCamera + _dyCamera));
        ft = _focalDistance / dy.z;
        pFocus = Point3f(0, 0, 0) + (ft * dy);
        ray->ryOrigin = Point3f(pLens.x, pLens.y, 0);
        ray->ryDirection = normalize(pFocus - ray->ryOrigin);
    } else {
        ray->rxOrigin = ray->ryOrigin = ray->ori;
        ray->rxDirection = normalize(Vector3f(pCamera) + _dxCamera);
        ray->ryDirection = normalize(Vector3f(pCamera) + _dyCamera);
    }
    ray->time = lerp(sample.time, shutterOpen, shutterClose);
    ray->medium = medium;
    *ray = cameraToWorld.exec(*ray);
    ray->hasDifferentials = true;
    return 1;
}

nebJson PerspectiveCamera::toJson() const {
    return nebJson();
}

Spectrum PerspectiveCamera::We(const paladin::Ray &ray, Point2f *pRaster2) const {
    // todo 双向方法里用到，暂时不实现
    return Spectrum(0);
}


void PerspectiveCamera::pdf_We(const Ray &ray, Float *pdfPos, Float *pdfDir) const {
    // todo 双向方法里用到，暂时不实现
}

Spectrum PerspectiveCamera::sample_Wi(const Interaction &ref, const Point2f &u,
                          Vector3f *wi, Float *pdf, Point2f *pRaster,
                          VisibilityTester *vis) const {
    // todo 双向方法里用到，暂时不实现
    return Spectrum(0.f);
}

//"param" : {
//    "shutterOpen" : 0,
//    "shutterClose" : 1,
//    "lensRadius" : 0,
//    "focalDistance" : 100,
//    "fov" : 45,
//    "lookAt" : [
//        [0,0,-5],
//        [0,0,0],
//        [0,1,0]
//    ],
//    "lookAtEnd" : [
//        [0,0,-5],
//        [0,0,0],
//        [0,1,0]
//    ]
//}
// lst = {Film}
CObject_ptr createPerspectiveCamera(const nebJson &param, const Arguments &lst) {
    cout << param.ToFormattedString();
    Float shutterOpen = param.GetValue("shutterOpen", 0.f);
    Float shutterClose = param.GetValue("shutterClose", 1.f);
    Float lensRadius = param.GetValue("lensRadius", 0.f);
    Float focalDistance = param.GetValue("focalDistance", 100.f);
    Float fov = param.GetValue("fov", 45);
    nebJson lookAtParam = param.GetValue("lookAt", nebJson());
    nebJson lookAtEndParam = param.GetValue("lookAtEnd", nebJson());
    
    
    return nullptr;
}

REGISTER("perspective", createPerspectiveCamera);

PALADIN_END
