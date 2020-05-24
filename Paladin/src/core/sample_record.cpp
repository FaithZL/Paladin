//
//  sample_record.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/5/21.
//

#include "sample_record.hpp"
#include "interaction.hpp"
#include "core/shape.hpp"

PALADIN_BEGIN




PositionSamplingRecord::PositionSamplingRecord(const SurfaceInteraction &it, EMeasure measure)
: pos(it.pos),
time(it.time),
normal(it.normal),
uv(it.uv),
measure(measure),
object(it.shape) {

}

void PositionSamplingRecord::updateSurface(const SurfaceInteraction &si) {
    pos = si.pos;
    time = si.time;
    normal = si.normal;
    uv = si.uv;
    object = si.shape;
}



DirectSamplingRecord::DirectSamplingRecord(const Interaction &refIt,const SurfaceInteraction &targetSi,  EMeasure measure)
: PositionSamplingRecord(targetSi, measure),
ref(refIt.pos),
refNormal(refIt.normal) {
    dir = pos - ref;
    dist = dir.length();
    
}

DirectSamplingRecord::DirectSamplingRecord(const Interaction &refIt, EMeasure measure)
: PositionSamplingRecord(),
ref(refIt.pos),
refNormal(refIt.normal),
dir(Vector3f(0,0,0)),
dist(0),
pdfDir(0) {
    measure = ESolidAngle;
}

void DirectSamplingRecord::updateTarget(const SurfaceInteraction &si) {
    updateSurface(si);
    dir = pos - ref;
    dist = dir.length();
    dir = normalize(dir);
    pdfDir = pdfPos * dist * dist / absDot(normal, -dir);
}

PALADIN_END
