//
//  filter.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/5/1.
//

#include "filter.h"


PALADIN_BEGIN

void Filter::fillFilterTable() {
   int offset = 0;
   for (int y = 0; y < _filterTableWidth; ++y) {
       for (int x = 0; x < _filterTableWidth; ++x) {
           Point2f p;
           p.x = (x + 0.5f) * radius.x / _filterTableWidth;
           p.y = (y + 0.5f) * radius.y / _filterTableWidth;
           _filterTable[offset] = evaluate(p);
           ++offset;
       }
   }
   _distribute2D.init(_filterTable, _filterTableWidth, _filterTableWidth);
}

Point2f Filter::getPixelOffset(const Point2f &u, Float * weight) const {
    
    Vector2f dis = u - centerPos;
    Vector2i sign(dis.x > 0 ? 1 : -1, dis.y > 0 ? 1 : -1);
    Vector2f tmp = abs(dis) * 2;
    Point2f pIn = (Point2f)tmp;
    Point2f ret = _distribute2D.sampleContinuous(pIn);
    int maxIdx = _filterTableWidth - 1;
//    *weight = _filterTable2D[(int)(ret.x * maxIdx)][(int)(ret.y * maxIdx)];
    *weight = 1;
    ret.x *= radius.x * sign.x;
    ret.y *= radius.y * sign.y;
    ret = ret + centerPos;
    return ret;
}

PALADIN_END
