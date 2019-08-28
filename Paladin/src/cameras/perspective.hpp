//
//  perspective.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef perspective_hpp
#define perspective_hpp

#include "header.h"
#include "camera.hpp"

PALADIN_BEGIN

/*
 透视相机
 所有生成的ray的起点为相机空间的原点，方向为原点到像平面的采样的方向
 根据高斯透镜方程 

 1/z' - 1/z = 1/f

 f为焦距，z'为物体通过透镜所成像的z坐标，z为物体z坐标

 如下图所示，
 两条平行入射光通过透镜之后交汇到点p(平行于这两条入射光的光线都会交汇到p点)
 透镜中心z = 0
 右边为z轴正方向

  -------------/\
              /  \
             |    |     p
              \  /
  -------------\/

  根据高斯透镜方程，可得当z = -∞ , z = f,符合我们的预期

  经过变换
  z' = fz/(f+z)
  如下图所示，
  z物体在z处，假设投影所成像在z'处
  x物体在x处，假设投影所成像在x'处

   x           /\
              /  \
   z         |    |     z'
              \  /
               \/       x'

  如果所成像的z'或x'不在film上，则会一定程度上的导致模糊
  准确的说法是，如果z与x不在焦平面上，则会导致模糊

  个人理解
  透视相机其实是对真实相机的简化，但是比小孔相机要真实一些，因为可以模拟失焦产生的景深效果

  相机的焦距与两个参数有关，一个是透镜的形状，还有就是透镜与film的距离
  透视相机已经弱化了这两个概念，只提供了焦距这个成员变量
 */
class PerspectiveCamera : public ProjectiveCamera {
public:
    
    PerspectiveCamera(const AnimatedTransform &CameraToWorld,
                      const AABB2f &screenWindow, Float shutterOpen,
                      Float shutterClose, Float lensRadius, Float focalDistance,
                      Float fov, Film *film, const Medium *medium);
    
    virtual Float generateRay(const CameraSample &sample, Ray *) const;
    
    virtual Float generateRayDifferential(const CameraSample &sample,
                                  RayDifferential *ray) const;
    
    virtual Spectrum we(const Ray &ray, Point2f *pRaster2 = nullptr) const;
    
    virtual void pdfWe(const Ray &ray, Float *pdfPos, Float *pdfDir) const;
    
    virtual Spectrum sampleWi(const Interaction &ref, const Point2f &sample,
                       Vector3f *wi, Float *pdf, Point2f *pRaster,
                       VisibilityTester *vis) const;
    
private:
    // 向x轴移动一个像素，对应相机空间的变化率
    Vector3f _dxCamera;
    // 向y轴移动一个像素，对应相机空间的变化率
    Vector3f _dyCamera;
    // z=1的情况下的film的面积
    Float _area;
};

PALADIN_END

#endif /* perspective_hpp */
