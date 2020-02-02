//
//  perspective.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "cameras/perspective.hpp"
#include "math/sampling.hpp"
#include "core/light.hpp"

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

nloJson PerspectiveCamera::toJson() const {
    return nloJson();
}

/**
 * 参考资料
 * http://www.pbr-book.org/3ed-2018/Light_Transport_III_Bidirectional_Methods/The_Path-Space_Measurement_Equation.html
 *
 * 该函数可以理解相机的响应函数
 * 第一个参数可以理解为相机生成的ray，ray的原定暂时理解为相机位置
 * 
 * 
 *                 dA cosθ
 * 重要表达式 dw = -----------
 *                   r^2
 *
 * p(w)dw = p(A)dA   (A为相平面面积)
 *
 *                   r^2
 * p(w) = p(A) * ------------  =  (r^2) / (A cosθ)
 *                   cosθ
 *
 * r = 1/cosθ
 *
 *              1
 * p(w) = -------------
 *         A (cosθ)^3
 *
 * 
 * ∫[A_lens] ∫[sphere] We(p,w) cosθ dw dA(p) = 1
 *
 * 上式可以转为以下表达式
 * 
 * ∫[A_lens] ∫[sphere] p_a(p) p_w(w) dw dA(p) = 1
 *
 * We(p,w) = (p_a(p) p_w(w) )/ cosθ
 *
 * 其中p_a(p)为透镜表面的PDF，p_w(w)为相机位置朝相平面的PDF
 *
 * We(p,w) = p_w(w) / (πr^2 cosθ)
 * 
 * 
 * @param  ray      暂时理解为ray的原点就是相机位置
 * @param  pRaster2 [description]
 * @return          [description]
 */
Spectrum PerspectiveCamera::We(const Ray &ray, Point2f *pRaster2) const {
    Transform c2w = cameraToWorld.interpolate(ray.time);
    Float cosTheta = dot(ray.dir, c2w.exec(Vector3f(0, 0, 1)));
    if (cosTheta <= 0) {
        return Spectrum(0);
    }
    // 计算前焦点
    // 对于有透镜的相机，计算焦平面的位置
    // 小孔相机计算z = 1平面
    // 焦点在世界空间的坐标
    Point3f pFocus = ray.at((_lensRadius > 0 ? _focalDistance : 1) / cosTheta);
    // 焦点在光栅空间的坐标
    Point3f pRaster = _rasterToCamera.getInverse().exec(c2w.getInverse().exec(pFocus));
    if (pRaster2) {
        *pRaster2 = Point2f(pRaster.x, pRaster.y);
    }
    // 如果超出感应器范围，则返回0
    AABB2i sampleBounds = film->getSampleBounds();
    if (pRaster.x > sampleBounds.pMax.x || pRaster.y > sampleBounds.pMax.y ||
        pRaster.x < sampleBounds.pMin.x || pRaster.y < sampleBounds.pMin.y) {
        return Spectrum(0);
    }
    
    Float lensArea = _lensRadius == 0 ? 1 : (Pi * _lensRadius * _lensRadius);
    Float cosTheta_2 = cosTheta * cosTheta;
    return Spectrum(1 / (_area * lensArea * cosTheta_2 * cosTheta_2));
}


void PerspectiveCamera::pdf_We(const Ray &ray, Float *pdfPos, Float *pdfDir) const {
    Transform c2w = cameraToWorld.interpolate(ray.time);
    Float cosTheta = dot(ray.dir, c2w.exec(Vector3f(0, 0, 1)));
    if (cosTheta <= 0) {
        *pdfPos = *pdfDir = 0;
        return;
    }
    // 计算前焦点
    // 对于有透镜的相机，计算焦平面的位置
    // 小孔相机计算z = 1平面
    // 焦点在世界空间的坐标
    Point3f pFocus = ray.at((_lensRadius > 0 ? _focalDistance : 1) / cosTheta);
    // 焦点在光栅空间的坐标
    Point3f pRaster = _rasterToCamera.getInverse().exec(c2w.getInverse().exec(pFocus));
    
    AABB2i sampleBounds = film->getSampleBounds();
    if (pRaster.x > sampleBounds.pMax.x || pRaster.y > sampleBounds.pMax.y ||
        pRaster.x < sampleBounds.pMin.x || pRaster.y < sampleBounds.pMin.y) {
        *pdfPos = *pdfDir = 0;
        return;
    }
    
    Float lensArea = _lensRadius != 0 ? (Pi * _lensRadius * _lensRadius) : 1;
    *pdfPos = 1 / lensArea;
    *pdfDir = 1 / (_area * cosTheta * cosTheta * cosTheta);
}

/**
 * 在ref点处采样相机的film
 * @param  ref     场景中的某个点
 * @param  u       均匀随机变量
 * @param  wi      采样生成方向，指向相机方向
 * @param  pdfDir  ref点采样film对应立体角空间中的概率密度函数
 * @param  pRaster 对应的光栅点
 * @param  vis     
 * @return         We函数值
 */
Spectrum PerspectiveCamera::sample_Wi(const Interaction &ref, const Point2f &u,
                          Vector3f *wi, Float *pdfDir, Point2f *pRaster,
                          VisibilityTester *vis) const {
    
    Point2f pLens = uniformSampleDisk(u);
    Point3f pLensWorld = cameraToWorld.exec(ref.time, Point3f(pLens.x, pLens.y, 0));
    Interaction pLensIntr(pLensWorld, ref.time, medium);
    pLensIntr.normal = cameraToWorld.exec(ref.time, Normal3f(0, 0, 1));
    
    *wi = pLensWorld - ref.pos;
    *vis = VisibilityTester(pLensIntr, ref);
    Float length = wi->length();
    *wi /= length;
    
    Float lensArea = _lensRadius != 0 ? (Pi * _lensRadius * _lensRadius) : 1;
    //                   r^2
    // p(w) = p(A) * ------------  =  (r^2) / (A cosθ)
    //                   cosθ
    *pdfDir = (length * length) / (absDot(pLensIntr.normal, *wi) * lensArea);
    
    return We(pLensIntr.spawnRay(-*wi), pRaster);
}

//"param" : {
//    "shutterOpen" : 0,
//    "shutterClose" : 1,
//    "lensRadius" : 0,
//    "focalDistance" : 100,
//    "fov" : 45,
//    "aspect" : 1,
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
CObject_ptr createPerspectiveCamera(const nloJson &param, const Arguments &lst) {
    Float shutterOpen = param.value("shutterOpen", 0.f);
    Float shutterClose = param.value("shutterClose", 1.f);
    Float lensRadius = param.value("lensRadius", 0.f);
    Float focalDistance = param.value("focalDistance", 100.f);
    Float fov = param.value("fov", 45);
    nloJson lookAtParam = param.value("lookAt", nloJson::object());
    nloJson lookAtEndParam = param.value("lookAtEnd", lookAtParam);

    
    auto iter = lst.begin();
    auto film = shared_ptr<Film>(dynamic_cast<Film *>(*iter));

    Float aspect = Float(film->fullResolution.x) / Float(film->fullResolution.y);
    
    AABB2f scrn;
    if (aspect > 1.f) {
        scrn.pMin.x = -aspect;
        scrn.pMax.x = aspect;
        scrn.pMin.y = -1.f;
        scrn.pMax.y = 1.f;
    } else {
        scrn.pMin.x = -1.f;
        scrn.pMax.x = 1.f;
        scrn.pMin.y = -1.f / aspect;
        scrn.pMax.y = 1.f / aspect;
    }
    
    Transform * lookAt = nullptr;
    Transform * lookAtEnd = nullptr;
    
    nloJson transform = param.value("transform", nloJson());
    if (!transform.is_null()) {
        lookAt = createTransform(transform);
        * lookAt = lookAt->getInverse();
        lookAtEnd = createTransform(transform);
        * lookAtEnd = lookAtEnd->getInverse();
    } else {
        lookAt = dynamic_cast<Transform *>(createLookAt(lookAtParam, {}));
        lookAtEnd = dynamic_cast<Transform *>(createLookAt(lookAtEndParam, {}));
    }
    
    AnimatedTransform animatedTransform(shared_ptr<const Transform>(lookAt->getInverse_ptr()),
                                        shutterOpen,
                                        shared_ptr<const Transform>(lookAtEnd->getInverse_ptr()),
                                        shutterClose);
    
    PerspectiveCamera * ret(new PerspectiveCamera(animatedTransform,
                                                  scrn,
                                                  shutterOpen,
                                                  shutterClose,
                                                  lensRadius,
                                                  focalDistance,
                                                  fov,
                                                  film,
                                                  nullptr));
    
    return ret;
}

REGISTER("perspective", createPerspectiveCamera);

PALADIN_END
