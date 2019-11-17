//
//  camera.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include "camera.hpp"


PALADIN_BEGIN

Camera::Camera(const AnimatedTransform &CameraToWorld, Float shutterOpen,
               Float shutterClose, shared_ptr<Film> film, const Medium *medium)
: cameraToWorld(CameraToWorld),
shutterOpen(shutterOpen),
shutterClose(shutterClose),
film(film),
medium(medium) {
    if (cameraToWorld.hasScale()) {

    }

}

Float Camera::generateRayDifferential(const CameraSample &sample, RayDifferential *rd) const {
    Float weight = generateRay(sample, rd);
    if (weight == 0) {
        return weight;
    }

    // 生成x方向辅助光线
    CameraSample sshift = sample;
    ++sshift.pFilm.x;
    Ray rx;
    Float weightX = generateRay(sshift, &rx);
    // todo 暂时不理解为何要这样要返回0
    if (weightX == 0) {
        return 0;
    }
    rd->rxOrigin = rx.ori;
    rd->rxDirection = rx.dir;
    
    // 生成y方向辅助光线
    --sshift.pFilm.x;
    ++sshift.pFilm.y;
    Ray ry;
    Float weightY = generateRay(sshift, &ry);
    rd->ryOrigin = ry.ori;
    rd->ryDirection = ry.dir;
    if (weightY == 0) {
        return 0;
    }
    
    return weight;
}


Spectrum Camera::We(const Ray &ray, Point2f *raster) const {
    COUT << "amera::We is not implemented.";
    assert(false);
    return Spectrum(0.f);
}

void Camera::pdf_We(const Ray &ray, Float *pdfPos, Float *pdfDir) const {
    COUT << "Camera::Pdf_We() is not implemented!";
    assert(false);
}

Spectrum Camera::sample_Wi(const Interaction &ref, const Point2f &u,
                           Vector3f *wi, Float *pdf, Point2f *pRaster,
                           VisibilityTester *vis) const {
    COUT << "Camera::Sample_Wi() is not implemented!";
    assert(false);
    return Spectrum(0.f);
}

PALADIN_END
