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
	DCHECK(threads.size() > 0 || maxThreadIndex() == 1);

	if (threads.empty() || count < chunkSize) {
		for (int64_t i = 0; i < count; ++i) {
			func(i);
		}
		return
	}

	ParallelForLoop loop(std::move(func), count, chunkSize, 0);
	workListMutex.lock();
	loop.next = workList;
	workList = &loop;
	workListMutex.unlock();

	std::unique_lock<std::mutex> lock(workListMutex);
    workListCondition.notify_all();
}

void parallelFor2D(std::function<void(Point2i)> func, const Point2i &count) {

}

static void workerThreadFunc(int tIndex, std::shared_ptr<Barrier> barrier) {
	ThreadIndex = tIndex;

	// 等待，最后一个子线程调用此函数之后，全部子线程同时开始往下执行
	barrier->wait();

	//每个线程各自释放掉barrier对象
	barrier->reset();

	// 以下逻辑涉及线程同步问题，上锁
	std::unique_lock<std::mutex> lock(workListMutex);
	while (!shutdownThreads) {
		if (reportWorkerStats) {
			if (--reporterCount ==) {
				reportDoneCondition.notify_one();
			}
			workListCondition.wait(lock);
		} else if (!workList) {
			// 如果没有任务需要执行，则等待
			workListCondition.wait(lock);
		} else {
			ParallelForLoop &loop = *workList;

			int64_t indexStart = loop.nextIndex;
			int64_t indexEnd = std::min(indexStart + loop.chunkSize, loop.maxIndex);

			// 这个循环结束的索引就是下个循环的开始，
			loop.nextIndex = indexEnd;
			if (loop.nextIndex == loop.maxIndex) {
				// 如果当前loop已经执行完毕，则执行下一个loop
				workList = loop.next;
			}

			++loop.activeWorkers;
			lock.unlock();
			// 执行[indexStart, indexEnd)区间内的索引
			for (int64_t index = indexStart; index < indexEnd; ++index) {
				if (loop.func1D) {
					loop.func1D(index);
				} else {
					loop.func2D(Point2i(index % loop.nX, index / loop.nX));
				}
			}
			lock.lock();
			--loop.activeWorkers;
			if (loop.finished()) {
				// 如果loop执行完毕，则唤醒所有线程
				workListCondition.notify_all();
			}
		}
	}
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
	{
		std::lock_guard<std::mutex> lock(workListMutex);
        shutdownThreads = true;
        workListCondition.notify_all();
	}
	for (std::thread &thread : threads) {
		thread.join();
	}
    threads.erase(threads.begin(), threads.end());
    shutdownThreads = false;
}

void mergeWorkerThreadStats() {

}

PALADIN_END
