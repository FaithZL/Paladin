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
    
protected:
    Point3f _pos;
    
    Normal3f _normal;
    
    Point2f _uv;
    
    EMeasure measure;
    
    Float _pdfPos;
public:
    
    inline const Point3f &pos() const {
        return _pos;
    }
    
    inline const Normal3f &normal() const {
        return _normal;
    }
    
    inline const Point2f &uv() const {
        return _uv;
    }
    
    inline Float pdfPos() const {
        return _pdfPos;
    }
    
    inline void setPdfPos(Float pdf) {
        _pdfPos = pdf;
    }
    
    Float time;
    
    const CObject * object;
    
    void updateSurface(const SurfaceInteraction &si);
    
    inline void setGeometry(const Point3f &pos,
                            const Normal3f &normal,
                            const Point2f &uv,
                            Float pdf = 0) {
        this->_pos = pos;
        this->_normal = normal;
        this->_uv = uv;
        this->_pdfPos = pdf;
    }
    
    const CObject * getObject() const {
        return object;
    }
    
    PositionSamplingRecord():
    _pos(Point3f(0,0,0)),
    time(0),
    _normal(Normal3f(0,0,0)),
    _pdfPos(0),
    _uv(Point2f(0,0)),
    object(nullptr) {
        
    }
    
    inline bool valid() const {
        return _normal.isZero();
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
private:
    
    const Point3f _ref;
    
    const Normal3f _refNormal;
    
    // ref指向pos,单位向量
    Vector3f _dir;
    
    Float _dist;
    
    Float _pdfDir;
    
public:
    
    inline const Point3f &ref() const {
        return _ref;
    }
    
    inline const Normal3f &refNormal() const {
        return _refNormal;
    }
    
    inline const Vector3f &dir() const {
        return _dir;
    }
    
    inline const Float pdfDir() const {
        return _pdfDir;
    }
    
    VisibilityTester getVisibilityTester() const;
    
    inline Float cosTargetTheta() const {
        return dot(-_dir, _normal);
    }
    
    void updateTarget(const SurfaceInteraction &si);
    
    inline void computeData() {
        _dir = _pos - _ref;
        _dist = _dir.length();
        _dir = normalize(_dir);
        _pdfDir = _pdfPos * _dist * _dist / absDot(_normal, -_dir);
        if (_dist == 0) {
            _pdfPos = _pdfDir = 0;
        } else if (std::isinf(_pdfDir)) {
            _pdfDir = 0;
        }
    }
    
    inline void updateTarget(const Point3f &pos,
                            const Normal3f &normal,
                            const Point2f &uv) {
        setGeometry(pos, normal, uv);
        computeData();
    }
    
    // 用于更新空中的点，主要是平行光与skylight的采样
    inline void updateTarget(const Vector3f &dir, Float pdfDir) {
        _pos = _ref + dir;
        _dir = normalize(dir);
        _pdfDir = pdfDir;
    }
    
    DirectSamplingRecord(const Interaction &refIt, EMeasure measure = ESolidAngle);
    
    DirectSamplingRecord(const Interaction &refIt,const SurfaceInteraction &targetSi,  EMeasure measure = ESolidAngle);
};

PALADIN_END

#endif /* sample_record_hpp */
