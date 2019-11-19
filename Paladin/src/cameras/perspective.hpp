//
//  perspective.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef perspective_hpp
#define perspective_hpp

#include "core/header.h"
#include "core/camera.hpp"

PALADIN_BEGIN

/**
 * 透视相机
 *
 * 用于模拟由于失焦产生的景深效果
 * 
 * 先简单介绍一下小孔相机，小孔成像众所周知
 * 物体在小孔的左侧，投影面在小孔右侧，物体会通过小孔投影在投影面(film)上，呈现一个倒着的实像
 * 
 * 为了简化计算，我们把film移到了与物体同一侧的近平面上，这样投影到film上的的物体的像就是正的了
 * 
 * pbrt所用的透视相机，稍微比小孔相机复杂些，把处在相机空间原点的小孔换成了透镜
 * 这样可以模拟由于失焦产生的景深效果
 * 
 * 所有生成的ray的起点为相机空间的原点，方向为原点到像平面的采样的方向
 * 根据高斯透镜方程 
 * 
 * 1/z' - 1/z = 1/f
 * 
 * f为焦距，z'为物体通过透镜所成像的z坐标，z为物体z坐标
 * 
 * 焦距的定义，如下所示
 * 如下图所示，
 * 5条平行入射光通过透镜之后交汇到点p(平行于这5条入射光的光线都会交汇到p点)
 * 透镜中心z = 0
 * 右边为z轴正方向
 * p点到透镜中心的距离为焦距
 * 
 *  -------------/\
 *  ------------/  \
 *  -----------(    )     p
 *  ------------\  /
 *  -------------\/
 *  
 *  根据高斯透镜方程，可得当z = -∞ , z = f,符合我们的预期
 *  
 *  经过变换
 *  z' = fz/(f+z)
 *  如下图所示，
 *  z物体在z处，假设投影所成像在z'处
 *  x物体在x处，假设投影所成像在x'处
 *
 *  x           /\
 *             /  \
 *  z         (    )     z'
 *             \  /
 *              \/       x'
 *
 * 如果所成像的z'或x'不在film上，则会一定程度上的导致模糊
 * 更加准确的说法是，如果z与x不在F平面上，则会导致zx在film上所成的像模糊
 *
 */
class PerspectiveCamera : public ProjectiveCamera {
public:
    
    PerspectiveCamera(const AnimatedTransform &CameraToWorld,
                      const AABB2f &screenWindow, Float shutterOpen,
                      Float shutterClose, Float lensRadius, Float focalDistance,
                      Float fov, shared_ptr<Film> film, const Medium *medium);
    
    virtual Float generateRay(const CameraSample &sample, Ray *) const override;
    
    virtual Float generateRayDifferential(const CameraSample &sample,
                                  RayDifferential *ray) const override;
    
    virtual Spectrum We(const Ray &ray, Point2f *pRaster2 = nullptr) const override;
    
    virtual void pdf_We(const Ray &ray, Float *pdfPos, Float *pdfDir) const override;
    
    virtual Spectrum sample_Wi(const Interaction &ref, const Point2f &sample,
                       Vector3f *wi, Float *pdf, Point2f *pRaster,
                       VisibilityTester *vis) const override;

    virtual nloJson toJson() const override;
    
private:
    // 向x轴移动一个像素，对应相机空间的变化率
    Vector3f _dxCamera;
    // 向y轴移动一个像素，对应相机空间的变化率
    Vector3f _dyCamera;
    // z=1的情况下的film的面积
    Float _area;
};

CObject_ptr createPerspectiveCamera(const nloJson &, const Arguments &);

PALADIN_END

#endif /* perspective_hpp */
