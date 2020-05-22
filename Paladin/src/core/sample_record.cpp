//
//  sample_record.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/5/21.
//

#include "sample_record.hpp"
#include "interaction.hpp"

PALADIN_BEGIN




PositionSamplingRecord::PositionSamplingRecord(const SurfaceInteraction &it, EMeasure measure)
: pos(it.pos),
time(it.time),
normal(it.normal),
uv(it.uv),
measure(measure) {

}

DirectSamplingRecord::DirectSamplingRecord(const Interaction &refIt,const SurfaceInteraction &targetSi,  EMeasure measure)
: PositionSamplingRecord(targetSi, measure),
ref(refIt.pos),
refNormal(refIt.normal) {
    
}


PALADIN_END
