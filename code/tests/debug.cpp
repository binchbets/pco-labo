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
        std::cout << "Not sorted...";
    }

    for (int i: array)
        std::cout << i << ' ';
}

int main(int argc, char **argv) {
    test(1, 20, 0);
}