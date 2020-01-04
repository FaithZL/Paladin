//
//  progressreporter.cpp
//  Paladin
//
//  Created by SATAN_Z on 2020/1/4.
//

#include "progressreporter.hpp"

PALADIN_BEGIN

ProgressReporter::ProgressReporter(const string &title, int64_t total)
: _title(title),
_totalWork(total),
_startTime(chrono::system_clock::now()) {
    _workDone = 0;
    _exitThread = false;
    
    std::shared_ptr<Barrier> barrier = std::make_shared<Barrier>(2);
    _updateThread = std::thread([this, barrier]() {
        barrier->wait();
        printProgress();
    });

    barrier->wait();
}

ProgressReporter::~ProgressReporter() {
    _workDone = _totalWork;
    _exitThread = true;
    _updateThread.join();
    printf("\n");
}

void ProgressReporter::printProgress() const {
    std::chrono::milliseconds sleepDuration(250);
    int bufLen = 100;
    unique_ptr<char[]> buf(new char[bufLen]);
    int iterCount = 0;
    while (!_exitThread) {
        this_thread::sleep_for(sleepDuration);
        ++iterCount;
        if (iterCount == 10)
            // Up to 0.5s after ~2.5s elapsed
            sleepDuration *= 2;
        else if (iterCount == 70)
            // Up to 1s after an additional ~30s have elapsed.
            sleepDuration *= 2;
        else if (iterCount == 520)
            // After 15m, jump up to 5s intervals
            sleepDuration *= 5;
        
        Float percentDone = Float(_workDone) / Float(_totalWork);
        Float seconds = elapsedMS() / 1000.f;
        Float estRemaining = seconds / percentDone - seconds;
        printf("progress is %.2f%%",percentDone * 100);
        if (percentDone == 1.f)
            printf(" (%.1fs)       ", seconds);
        else if (!std::isinf(estRemaining))
            printf(" (%.1fs|%.1fs)  ", seconds,
                   std::max((Float)0., estRemaining));
        else
            printf(" (%.1fs|?s)  ", seconds);
//        string content;
//        content = StringPrintf("progress is %5.2f%%, (%5.2fs|%5.2fs)",
//                                percentDone * 100, seconds, estRemaining);
        fflush(stdout);
    }
}

PALADIN_END
