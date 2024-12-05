#ifndef QUICKSORT_H
#define QUICKSORT_H

#include <iostream>

#include <pcosynchro/pcothread.h>
#include <pcosynchro/pcomutex.h>
#include <pcosynchro/pcoconditionvariable.h>
#include <queue>
#include <optional>
#include <utility>
#include "multithreadedsort.h"

/**
 * @brief A unit of work in the Multithreaded Quicksort implementation
 */
template<typename T>
class Task {
public:
    /**
     * @param array The array to sort
     * @param lo The starting index
     * @param hi The end index
     */
    Task(std::vector<T> &array, size_t lo, size_t hi) : array(array), lo(lo),  hi(hi) {}    

    std::vector<T> &array;
    const size_t hi, lo;
};

/**
 * @brief The Quicksort class implements the multi-threaded Quicksort algorithm.
 */
template<typename T>
class Quicksort : public MultithreadedSort<T> {
public:
    /**
     * @param nbThreads The number of workers threads to create that will run the quicksort.
     * @throws std::invalid_argument If nbThreads is 0.
     */
    Quicksort(unsigned int nbThreads) : MultithreadedSort<T>(nbThreads) {
        if (nbThreads == 0) {
            throw std::invalid_argument("nbThreads cannot be 0");
        }
    }

    /**
     * @brief sort Manages the threads to sort the given sequence.
     * @param array is the sequence to sort
     */
    void sort(std::vector<T> &array) override {
        std::vector<PcoThread> workers;
        workers.reserve(this->nbThreads);

        // Add the initial task
        // We do this before creating workers. This saves us a notifyOne call.
        queue.emplace(array, 0, array.size() - 1);

        for (size_t i = 0; i < this->nbThreads; i++) {
            workers.emplace_back(&Quicksort::do_sort, this);
        }

        // We wait until all threads are done
        while (!isDone) {
            isDoneCondition.wait(&isDoneMutex);
        }

        for (auto &worker: workers) {
            worker.requestStop();
        }
        std::cout << "queueNotEmpty about to be notified" << std::endl;
        // We notify all threads in order to terminate them gracefully
        queueNotEmpty.notifyAll();

        for (auto &worker: workers) {
            worker.join();
        }
    }

private:

    /**
     * The queue of tasks that will be processed by the worker threads
     */
    std::queue<Task<T>> queue;

    /**
     * The number of threads currently processing a task
     */
    int active = 0;

    /**
     * A mutex to be used by isDoneCondition.
     */
    PcoMutex isDoneMutex;
    /**
     * The condition variable to notify the parent that the sort has finished
     */
    PcoConditionVariable isDoneCondition;

    /**
     * Is done holds a notification that the processing is finished, and we are ready to cleanup and finish the sort
     */
     bool isDone = false;

    /**
     * A mutex to protect the queue.
     */
    PcoMutex mutex;
    /**
     * A condition variable to park the threads that waits for a new task to process.
     */
    PcoConditionVariable queueNotEmpty;

    /**
     * do_sort is responsible for consuming available tasks and producing new ones.
     */
    void do_sort() {
        while (!PcoThread::thisThread()->stopRequested()) {
            mutex.lock();
            while (queue.empty()) {
                if (PcoThread::thisThread()->stopRequested()) {
                    mutex.unlock();
                    return;
                }

                queueNotEmpty.wait(&mutex);
            }
            // Get the task to compute
            Task<T> task = queue.front();
            queue.pop();
            active++;
            mutex.unlock();

            if (task.lo >= task.hi || task.lo < 0) {
                // When lo and hi have crossed, we have met the stopping condition.
                finish();
                continue;
            }

            size_t p = partition(task.array, task.lo, task.hi);

            mutex.lock();
            queue.emplace(task.array, task.lo, p==0 ? 0: p -1);
            queue.emplace(task.array, p + 1, task.hi);
            mutex.unlock();

            queueNotEmpty.notifyOne();
            queueNotEmpty.notifyOne();

            finish();
        }
    }

    /**
     * Finish announces that a unit of work was finished. It also handles and starts the main process if all
     * units of work are done (no more active workers, and the queue is empty)
     */
    void finish() {
        mutex.lock();
        active--;
        if (active <= 0 && queue.empty()) {
            mutex.unlock();
            // Notify itself that it should stop. Otherwise we may get a race condition
            PcoThread::thisThread()->requestStop();

            isDoneMutex.lock();
            isDone = true;
            isDoneMutex.unlock();

            isDoneCondition.notifyOne();
            // The last thread has computed its task, we are done.

            return;
        }
        mutex.unlock();
    }

    size_t partition(std::vector<T> &array, size_t lo, size_t hi) {
        T pivot = array[hi];

        size_t i = lo;

        for (size_t j = lo; j < hi; j++) {
            if (array[j] <= pivot) {
                std::swap(array[i], array[j]);
                i++;
            }
        }

        std::swap(array[i], array[hi]);

        return i;
    }
};


#endif // QUICKSORT_H
