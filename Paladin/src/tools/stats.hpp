//
//  stats.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/1/5.
//

#ifndef statistics_hpp
#define statistics_hpp

#include "core/header.h"

PALADIN_BEGIN

class Stats {
    
public:
    
    Stats() {
        _pathLenMax = 0;
        _nIntersect = 0;
        _nOccluded = 0;
        _nTriangle = 0;
        _pathNum = 0;
        _totalPathLen = 0;
        _pathLenMax = 0;
    }
    
    static Stats * getInstance();
    
    inline uint64_t getTriangleNum() const {
        return _nTriangle;
    }
    
    inline void addTriangle(uint64_t num) {
        _nTriangle += num;
    }
    
    inline void addIntersectTestNum(uint64_t num = 1) {
        _nIntersect += num;
    }
    
    inline uint64_t getIntersectTestNum() const {
        return _nIntersect;
    }
    
    inline void addOccludedTestNum(uint64_t num = 1) {
        _nOccluded += num;
    }
    
    inline uint64_t getOccludedTestNum() const {
        return _nOccluded;
    }
    
    inline void addPathLen(uint64_t num) {
        if (num > _pathLenMax) {
            _pathLenMax = num;
        }
        _totalPathLen += num;
        _pathNum += 1;
    }
    
    inline Float getAveragePathLen() const {
        return _totalPathLen / (Float)_pathNum;
    }
    
    inline uint64_t getMaxPathLen() const {
        return _pathLenMax;
    }
    
private:
    
    std::atomic<uint64_t> _nIntersect;
    
    std::atomic<uint64_t> _nOccluded;
    
    std::atomic<uint64_t> _pathLenMax;
    
    std::atomic<uint64_t> _totalPathLen;
    
    std::atomic<uint64_t> _pathNum;
    
    uint64_t _nTriangle;
    
    static Stats * s_stats;
};

PALADIN_END

#endif /* stats_hpp */
