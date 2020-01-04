//
//  progressreporter.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/1/4.
//

#ifndef progressreporter_hpp
#define progressreporter_hpp

#include "core/header.h"
#include "parallel.hpp"

PALADIN_BEGIN

USING_STD

class ProgressReporter {
    
public:
    ProgressReporter(const string &title, int64_t total);
    
    ~ProgressReporter();
    
    void update(int64_t num = 1) {
        if (num != 0) {
            _workDone += num;
        }
    }
    
    void done() {
        _workDone = _totalWork;
    }
    
    Float elapsedMS() const {
        chrono::system_clock::time_point now = chrono::system_clock::now();
        int64_t elapsedMS = chrono::duration_cast<chrono::milliseconds>(now - _startTime).count();
        return (Float)elapsedMS;
    }
    
private:
    
    void printProgress() const;
    
    const string _title;
    const int64_t _totalWork;
    const chrono::system_clock::time_point _startTime;
    atomic<int64_t> _workDone;
    atomic<bool> _exitThread;
    thread _updateThread;
};

PALADIN_END

#endif /* progressreporter_hpp */
