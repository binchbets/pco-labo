//
// Created by red on 12/3/24.
//

#include <gtest/gtest.h>
#include "quicksort.h"
#include "utils.h"

void test(int nbThreads, int size, int seed) {
    Quicksort<int> sorter(nbThreads);
    std::vector<int> array = generateSequence(size, seed);
    sorter.sort(array);

    if (!isSorted(array)) {
        std::cout << "Not sorted..." << std::endl;
    } else {
        std::cout << "Sorted :)" << std::endl;
    }
}

int main(int argc, char **argv) {
    test(4, 10'000'000, 0);
}