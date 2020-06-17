//
//  sample_record.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/5/21.
//

#include "sample_record.hpp"
#include "interaction.hpp"
#include "core/shape.hpp"
#include "core/light.hpp"

PALADIN_BEGIN




PositionSamplingRecord::PositionSamplingRecord(const SurfaceInteraction &it, EMeasure measure)
: _pos(it.pos),
time(it.time),
_normal(it.normal),
_uv(it.uv),
measure(measure),
_pdfPos(it.shape->pdfPos()),
object(it.shape) {

}

void PositionSamplingRecord::updateSurface(const SurfaceInteraction &si) {
    _pos = si.pos;
    time = si.time;
    _normal = si.normal;
    _uv = si.uv;
    _pdfPos = si.shape->pdfPos();
}



DirectSamplingRecord::DirectSamplingRecord(const Interaction &refIt,const SurfaceInteraction &targetSi,  EMeasure measure)
: PositionSamplingRecord(targetSi, measure),
_ref(refIt.pos),
_refNormal(refIt.normal),
refInside(refIt.mediumInterface.inside),
refOutside(refIt.mediumInterface.outside) {
    measure = ESolidAngle;
    computeData();
}

DirectSamplingRecord::DirectSamplingRecord(const Interaction &refIt, EMeasure measure)
: PositionSamplingRecord(),
_ref(refIt.pos),
_refNormal(refIt.normal),
_dir(Vector3f(0,0,0)),
_dist(-1),
_pdfDir(-1),
refInside(refIt.mediumInterface.inside),
refOutside(refIt.mediumInterface.outside),
checkOccluded(true) {
    this->measure = ESolidAngle;
}

void DirectSamplingRecord::updateTarget(const SurfaceInteraction &si) {
    updateSurface(si);
    computeData();
}

bool DirectSamplingRecord::unoccluded(const Scene &scene) const {
    auto vis = getVisibilityTester();
    return vis.unoccluded(scene);
}

Spectrum DirectSamplingRecord::Tr(const Scene &scene, Sampler &sampler) const {
    auto vis = getVisibilityTester();
    return vis.Tr(scene, sampler);
}

VisibilityTester DirectSamplingRecord::getVisibilityTester() const {
    Normal3f refN = _refNormal.isZero() ? Normal3f(_dir) : _refNormal;
    Interaction refIt(_ref, refN, time, MediumInterface(refInside, refOutside));
    Normal3f targetNormal = _normal.isZero() ? Normal3f(-_dir) : _normal;
    Interaction targetIt(_pos, targetNormal);
    auto vis = VisibilityTester(refIt, targetIt);
    return vis;
}

void DirectSamplingRecord::computeData() {
    TRY_PROFILE(Prof::shapePdfDir)
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

PALADIN_END
