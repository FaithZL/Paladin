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
    SuspendProfiler();
    std::shared_ptr<Barrier> barrier = std::make_shared<Barrier>(2);
    _updateThread = std::thread([this, barrier]() {
        ProfilerWorkerThreadInit();
        ProfilerState = 0;
        
        barrier->wait();
        printProgress();
    });
    
    barrier->wait();
    
    ResumeProfiler();
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
    snprintf(buf.get(), bufLen, "\r%s: ", _title.c_str());
    int iterCount = 0;
    fputs(buf.get(), stdout);
    fflush(stdout);
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
        fputs(buf.get(), stdout);
        printf("progress is %.2f%%",percentDone * 100);
        if (percentDone == 1.f)
            printf(" (%.2fs)       ", seconds);
        else if (!std::isinf(estRemaining))
            printf(" (%.2fs|%.2fs)  ", seconds,
                   std::max((Float)0., estRemaining));
        else
            printf(" (%.2fs|?s)  ", seconds);
        fflush(stdout);
    }
}

PALADIN_END
