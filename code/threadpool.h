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
        : maxThreadCount(maxThreadCount), maxNbWaiting(maxNbWaiting), idleTimeout(idleTimeout), runnables(), threads(), availableThreads(), availableThread(), notEmpty(), notFull() {
        threads.reserve(maxThreadCount);
    }

    ~ThreadPool() {
        std::cout << "destructor called" << std::endl;

        // TODO: Do we want to block on each call to `runnable->cancelRun()` ?
        while (!runnables.empty()) {
            std::unique_ptr<Runnable> runnable = std::move(runnables.front());
            runnables.pop();
            runnable->cancelRun();
        }

        // TODO : End smoothly
        for (PcoThread* thread : threads) {
            thread->join();
            delete thread;
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

        std::cout << "starting" << std::endl;

        if (runnables.size() >= maxNbWaiting) {
            std::cout << "too many runnables queued, canceling runnable" << std::endl;
            runnable->cancelRun();
            return false;
        }

        if (availableThreads.empty()) {
            if (threads.size() < maxThreadCount) {
                std::cout << "creating new thread" << std::endl;

                // We can create a new thread
                PcoThread* thread = new PcoThread(&ThreadPool::run, this);

                std::cout << "adding new thread to containers" << std::endl;

                availableThreads.push(thread);
                threads.push_back(thread);
            } else {
                // We reached the max number of threads, wait until one is available.
                wait(availableThread);
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
            signal(availableThread);

            // TODO: Should probably be an optional.
            // We either get the runnable or none if the thread pool should be stopped.
            std::unique_ptr<Runnable> runnable = get();

            std::cout << "running runnable" << std::endl;
            runnable->run();
        }
    }

    std::unique_ptr<Runnable> get() {
        monitorIn();

        if (runnables.empty()) {
            wait(notEmpty);
        }

        std::unique_ptr<Runnable> runnable = std::move(runnables.front());
        runnables.pop();

        signal(notFull);

        std::cout << "got runnable" << std::endl;

        monitorOut();
        return runnable;
    }

    /* Returns the number of currently running threads. They do not need to be executing a task,
     * just to be alive.
     */
    size_t currentNbThreads() {
        // TODO
        monitorIn();
        size_t count = availableThreads.size();
        monitorOut();

        return count;
    }

private:

    size_t maxThreadCount;
    size_t maxNbWaiting;
    std::chrono::milliseconds idleTimeout;

    /**
     * Used to check whether the queue of runnables is full.
     */
    Condition notFull;

    /**
     * Used to check whether the queue of runnables is empty
     */
    Condition notEmpty;

    /**
     * Used to check whether a thread is available to perform a runnable.
     */
    Condition availableThread;

    /**
     * List of runnables that
     */
    std::queue<std::unique_ptr<Runnable>> runnables;

    std::queue<PcoThread*> availableThreads;

    std::vector<PcoThread*> threads;
};

#endif // THREADPOOL_H
