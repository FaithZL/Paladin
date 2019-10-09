//
//  parallel.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/8/21.
//

#include "parallel.hpp"
#include <thread>

PALADIN_BEGIN

static std::vector<std::thread> threads;
static bool shutdownThreads = false;

static ParallelForLoop *workList = nullptr;
static std::mutex workListMutex;

static std::atomic<bool> reportWorkerStats{false};

static std::atomic<int> reporterCount;

static std::condition_variable reportDoneCondition;
static std::mutex reportDoneMutex;
thread_local int ThreadIndex = 0;

void parallelFor(std::function<void(int64_t)> func, int64_t count, int chunkSize) {

}

void parallelFor2D(std::function<void(Point2i)> func, const Point2i &count) {

}

static void workerThreadFunc(int tIndex, std::shared_ptr<Barrier> barrier) {
	
}

void parallelInit() {
	CHECK_EQ(threads.size(), 0);
	int nThreads = maxThreadIndex();
	ThreadIndex = 0;

	std::shared_ptr<Barrier> barrier = std::make_shared<Barrier>(nThreads);

	for (int i = 0; i < nThreads - 1; ++i) {
        threads.push_back(std::thread(workerThreadFunc, i + 1, barrier));	
	}

	barrier->wait();
}

void parallelCleanup() {
	if (threads.empty()) {
		return;
	} 
}

void mergeWorkerThreadStats() {

}

PALADIN_END
