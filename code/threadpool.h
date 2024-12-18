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
        : maxThreadCount(maxThreadCount), maxNbWaiting(maxNbWaiting), idleTimeout(idleTimeout), shouldStop(false), runningThreads(0), runnables(), threads(), notEmpty() {
        threads.reserve(maxThreadCount);
    }

    ~ThreadPool() {
        std::cout << "destructor called" << std::endl;

        monitorIn();

        shouldStop = true;

        // TODO: Do we want to block on each call to `runnable->cancelRun()` ?
        // while (!runnables.empty()) {
        //    std::unique_ptr<Runnable> runnable = std::move(runnables.front());
        //    runnables.pop();
        //    runnable->cancelRun();
        // }

        std::cout << "signaling all threads" << std::endl;

        for (size_t i = 0; i < threads.size(); i++) {
            signal(notEmpty);
        }

        monitorOut();

        std::cout << "joining threads" << std::endl;

        // TODO : End smoothly
        for (PcoThread& thread : threads) {
            thread.join();
        }

        std::cout << "destructor done" << std::endl;
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

        std::cout << "starting" << std::endl;

        if (runnables.size() >= maxNbWaiting) {
            std::cout << "too many runnables queued, canceling runnable" << std::endl;
            runnable->cancelRun();
            monitorOut();

            return false;
        }

        if (runningThreads == threads.size()) {
            if (threads.size() < maxThreadCount) {
                std::cout << "creating new thread" << std::endl;

                // We can create a new thread
                threads.emplace_back(&ThreadPool::run, this);
                runningThreads++;
            }
        }

        std::cout << "adding new runnable to queue" << std::endl;

        runnables.push(std::move(runnable));

        signal(notEmpty);

        monitorOut();

        std::cout << "return true" << std::endl;

        return true;
    }

    /**
     * This method is called by each thread to perform a Runnable task.
     */
    void run() {
        std::cout << "new thread created" << std::endl;

        while (!PcoThread::thisThread()->stopRequested()) {
            // TODO: Should probably be an optional.
            // We either get the runnable or none if the thread pool should be stopped.
            std::optional<std::unique_ptr<Runnable>> runnable = get();

            if (!runnable.has_value()) {
                return;
            }

            std::cout << "running runnable" << std::endl;
            runnable.value()->run();
            std::cout << "runnable done" << std::endl;
        }
    }

    std::optional<std::unique_ptr<Runnable>> get() {
        monitorIn();

        std::cout << "get runnable" << std::endl;

        if (runnables.empty()) {
            wait(notEmpty);
        }

        if (shouldStop) {
            std::cout << "should stop" << std::endl;
            monitorOut();
            return std::nullopt;
        }

        std::unique_ptr<Runnable> runnable = std::move(runnables.front());
        runnables.pop();

        std::cout << "got runnable" << std::endl;

        monitorOut();
        return runnable;
    }

    /* Returns the number of currently running threads. They do not need to be executing a task,
     * just to be alive.
     */
    size_t currentNbThreads() {
        // TODO
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

    bool shouldStop;

    size_t runningThreads;

    /**
     * List of runnables that
     */
    std::queue<std::unique_ptr<Runnable>> runnables;

    std::vector<PcoThread> threads;
};

#endif // THREADPOOL_H
