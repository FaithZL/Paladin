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
    
    static Stats * getInstance();
    
    inline uint64_t getTriangleNum() const {
        return _nTriangle;
    }
    
    inline void addTriangle(uint64_t num) {
        _nTriangle += num;
    }
    
private:
    
    uint64_t _nTriangle;
    
    static Stats * s_stats;
};

PALADIN_END

#endif /* stats_hpp */
