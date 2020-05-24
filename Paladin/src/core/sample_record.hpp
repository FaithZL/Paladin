//
//  sample_record.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/5/21.
//

#ifndef sample_record_hpp
#define sample_record_hpp

#include "core/header.h"

PALADIN_BEGIN

enum EMeasure {
    /// Invalid measure
    EInvalidMeasure = 0,
    /// Solid angle measure
    ESolidAngle = 1,
    /// Length measure
    ELength = 2,
    /// Area measure
    EArea = 3,
    /// Discrete measure
    EDiscrete = 4
};

struct PositionSamplingRecord {
    Point3f pos;
    
    Float time;
    
    Normal3f normal;
    
    Float pdfPos;
    
    Point2f uv;
    
    const CObject * object;
    
    EMeasure measure;
    
    void updateSurface(const SurfaceInteraction &si);
    
    PositionSamplingRecord():
    pos(Point3f(0,0,0)),
    time(0),
    normal(Normal3f(0,0,0)),
    pdfPos(0),
    uv(Point2f(0,0)),
    object(nullptr) {
        
    }
    
    PositionSamplingRecord(const SurfaceInteraction &it,
                                  EMeasure measure = EArea);
};

struct DirectionSamplingRecord {
    Vector3f dir;
    
    /// Probability density at the sample
    Float pdf;

    /// Measure associated with the density function
    EMeasure measure;
    
    inline DirectionSamplingRecord(const Vector3f &d,
        EMeasure measure = ESolidAngle)
    : dir(d), measure(measure) {
        
    }
    
};

struct DirectSamplingRecord : public PositionSamplingRecord {
    const Point3f ref;
    
    const Normal3f refNormal;
    
    // ref指向pos,单位向量
    Vector3f dir;
    
    Float dist;
    
    Float pdfDir;
    
    void updateTarget(const SurfaceInteraction &si);
    
    DirectSamplingRecord(const Interaction &refIt, EMeasure measure = ESolidAngle);
    
    DirectSamplingRecord(const Interaction &refIt,const SurfaceInteraction &targetSi,  EMeasure measure = ESolidAngle);
};

PALADIN_END

#endif /* sample_record_hpp */
