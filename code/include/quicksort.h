#ifndef QUICKSORT_H
#define QUICKSORT_H

#include <iostream>

#include <pcosynchro/pcothread.h>
#include <pcosynchro/pcomutex.h>
#include <pcosynchro/pcoconditionvariable.h>
#include "multithreadedsort.h"

template<typename T>
class Task {
public:
    Task(std::vector<T>& array, T hi, T lo): array(array), hi(hi), lo(lo) {}

private:
    std::vector<T>& array;
    T hi, lo;
};

/**
 * @brief The Quicksort class implements the multi-threaded Quicksort algorithm.
 */
template<typename T>
class Quicksort: public MultithreadedSort<T>
{
public:
    Quicksort(unsigned int nbThreads) : MultithreadedSort<T>(nbThreads) {}

    /**
     * @brief sort Manages the threads to sort the given sequence.
     * @param array is the sequence to sort
     */
    void sort(std::vector<T>& array) override {
        // TODO
        std::vector<PcoThread> workers;
        workers.reserve(this->nbThreads);

        for (size_t i = 0; i < this->nbThreads; i++) {
            workers.emplace_back(&Quicksort::do_sort, this);
        }

        // Do the thing

        for (auto & worker : workers) {
            worker.join();
        }
    }
private:

    void do_sort() {
    }

    size_t partition(std::vector<T>& array, T hi, T lo) {
        T pivot = array[hi];

        T i = lo;

        for (T j = lo; j < hi - 1; j++) {
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
