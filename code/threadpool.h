#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <iostream>
#include <stack>
#include <vector>
#include <chrono>
#include <cassert>
#include <pcosynchro/pcologger.h>
#include <pcosynchro/pcothread.h>
#include <pcosynchro/pcohoaremonitor.h>

class Runnable {
public:
    virtual ~Runnable() = default;
    virtual void run() = 0;
    virtual void cancelRun() = 0;
    virtual std::string id() = 0;
};

class ThreadPool : public PcoHoareMonitor {
public:
    ThreadPool(int maxThreadCount, int maxNbWaiting, std::chrono::milliseconds idleTimeout)
        : maxThreadCount(maxThreadCount), maxNbWaiting(maxNbWaiting), idleTimeout(idleTimeout),
        shouldStop(false), runningThreads(0), runnables(), threads(), notEmpty(), timeoutThread(&ThreadPool::timeout, this) {
        threads.reserve(maxThreadCount);
    }

    ~ThreadPool() {
        monitorIn();

        shouldStop = true;

        // We cancel any remaining runnables that still exists.
        while (!runnables.empty()) {
            std::unique_ptr<Runnable> runnable = std::move(runnables.front());
            runnables.pop();
            runnable->cancelRun();
        }

        for (size_t i = 0; i < threads.size(); i++) {
            signal(notEmpty);
        }

        timeoutThread.requestStop();
        signal(threadWaiting);

        monitorOut();

        timeoutThread.join();

        // TODO : End smoothly
        for (std::unique_ptr<PcoThread> &thread : threads) {
            thread->join();
        }
    }

    /*
     * Start a runnable. If a thread in the pool is avaible, assign the
     * runnable to it. If no thread is available but the pool can grow, create a new
     * pool thread and assign the runnable to it. If no thread is available and the
     * pool is at max capacity and there are less than maxNbWaiting threads waiting,
     * block the caller until a thread becomes available again, and else do not run the runnable.
     * If the runnable has been started, returns true, and else (the last case), return false.
     */
    bool start(std::unique_ptr<Runnable> runnable) {
        // TODO
        monitorIn();

        if (runnables.size() >= maxNbWaiting) {
            runnable->cancelRun();
            monitorOut();

            return false;
        }

        // We can create a new thread if all threads are running, and we haven't reached the thread limit
        if (runningThreads == threads.size() && threads.size() < maxThreadCount) {
            // We can create a new thread
            threads.emplace_back(new PcoThread(&ThreadPool::run, this));
            runningThreads++;
        }

        runnables.push(std::move(runnable));
        signal(notEmpty);

        monitorOut();

        return true;
    }

    /**
     * This method is called by each thread to perform a Runnable task.
     */
    void run() {
        while (!PcoThread::thisThread()->stopRequested()) {
            // TODO: Should probably be an optional.
            // We either get the runnable or none if the thread pool should be stopped.
            std::optional<std::unique_ptr<Runnable>> runnable = get();

            if (!runnable.has_value()) {
                runningThreads--;
                return;
            }

            runnable.value()->run();
        }
    }

    void timeout() {
        // TODO: Add Condition for waiting threads

        while(!PcoThread::thisThread()->stopRequested()) {
            monitorIn();

            if (timestamps.empty()) {
                wait(threadWaiting);

                if (PcoThread::thisThread()->stopRequested()) {
                    monitorOut();
                    return;
                }
            }

            std::chrono::milliseconds timeout = idleTimeout - time_diff(timestamps.front());
            monitorOut();

            PcoThread::usleep(timeout.count() * (uint64_t) 1000);

            monitorIn();
            // We waited for quite some time, so it may be possible that the timestamp we waited on has finished.
            // Also, we may have more than one thread that waits
            while (!timestamps.empty() && time_diff(timestamps.front()) > timeout) {
                // The signaled thread will be the first one to have called `wait(notEmpty)`, which is the thread that
                // has waited for the longest time.
                signal(notEmpty);
            }
            monitorOut();
        }
    }

    std::chrono::milliseconds time_diff(std::chrono::high_resolution_clock::time_point other) {
        auto diff = std::chrono::high_resolution_clock::now() - other;
        return std::chrono::duration_cast<std::chrono::milliseconds>(diff);
    }

    std::optional<std::unique_ptr<Runnable>> get() {
        monitorIn();

        if (runnables.empty()) {
            // We push the current time into the timestamps queue. The queue will be sorted as any new timestamp will be
            // greater than the ones that are already in the queue.
            timestamps.push(std::chrono::high_resolution_clock::now());
            // Announce to the timout thread that they can get started
            signal(threadWaiting);
            // Then wait (while possibly being waked by the timout task)
            wait(notEmpty);

            timestamps.pop();
        }

        // We might have an empty runnables queue if the thread has been awakened by the timeout.
        if (runnables.empty() || shouldStop) {
            monitorOut();
            return std::nullopt;
        }

        std::unique_ptr<Runnable> runnable = std::move(runnables.front());
        runnables.pop();

        monitorOut();
        return runnable;
    }

    /* Returns the number of currently running threads. They do not need to be executing a task,
     * just to be alive.
     */
    size_t currentNbThreads() {
        return runningThreads;
    }

private:

    size_t maxThreadCount;
    size_t maxNbWaiting;
    std::chrono::milliseconds idleTimeout;


    /**
     * Used to check whether the queue of runnables is empty
     */
    Condition notEmpty;

    /**
     * Used to indicate to the timeout thread that at least a thread is waiting.
     */
    Condition threadWaiting;

    bool shouldStop;

    size_t runningThreads;

    /**
     * List of runnables that
     */
    std::queue<std::unique_ptr<Runnable>> runnables;

    std::vector<std::unique_ptr<PcoThread>> threads;

    std::queue<std::chrono::high_resolution_clock::time_point> timestamps;

    PcoThread timeoutThread;
};

#endif // THREADPOOL_H
