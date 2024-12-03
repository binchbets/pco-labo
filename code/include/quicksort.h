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

template<typename T>
class Task {
public:
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
    Quicksort(unsigned int nbThreads) : MultithreadedSort<T>(nbThreads) {}


    /**
     * @brief sort Manages the threads to sort the given sequence.
     * @param array is the sequence to sort
     */
    void sort(std::vector<T> &array) override {
        std::vector<PcoThread> workers;
        workers.reserve(this->nbThreads);

        std::cout << "Letzgo";

        for (size_t i = 0; i < this->nbThreads; i++) {
            workers.emplace_back(&Quicksort::do_sort, this);
        }

        // Put the initial work
        put(Task(array, 0, array.size() - 1));


        poisonMutex.lock();
        while(!poison) {
            poisonCond.wait(&poisonMutex);
        }
        poisonMutex.unlock();

        // Everything has

        for (auto &worker: workers) {
            worker.join();
        }
    }

private:

    std::queue<Task<T>> queue;

    int active = 0;

    PcoMutex poisonMutex;
    PcoConditionVariable poisonCond;
    bool poison = false;

    PcoMutex mutex;
    PcoConditionVariable condVar;


    void put(Task<T> task) {
        mutex.lock();

        queue.push(task);

        mutex.unlock();

        condVar.notifyOne();
    }

    bool isPoisoned() {
        poisonMutex.lock();
        bool value = poison;
        poisonMutex.unlock();

        return value;
    }


    void do_sort() {
        while (!isPoisoned()) {
            mutex.lock();

            while (queue.empty()) {
                condVar.wait(&mutex);
            }

            // Stop early.
            if (isPoisoned()) {
                // Note, we don't decrement active, because it was poisoned.
                mutex.unlock();

                return;
            }

            Task<T> task = queue.front();
            queue.pop();

            active++;

            mutex.unlock();


            if (task.lo >= task.hi || task.lo < 0) {
                // We will always finish in this case (otherwise we would create new values)
                finish();
                continue;
            }

            size_t p = partition(task.array, task.lo, task.hi);

            put(Task(task.array, task.lo, p==0 ? 0: p -1));
            put(Task(task.array, p + 1, task.hi));

            finish();
        }
    }

    void finish() {
        mutex.lock();
        active--;

        if (active <= 0 && queue.empty()) {
            mutex.unlock();

            poisonMutex.lock();
            poison = true;
            poisonMutex.unlock();

            condVar.notifyAll();
            poisonCond.notifyOne();
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
