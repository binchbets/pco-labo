#include <gtest/gtest.h>

#include "quicksort.h"
#include "utils.h"

/**
 * @brief test Generates a random sequence of specified size and sorts it with Quicksort using N threads.
 * @param nbThreads number of threads to use to sort the sequence
 * @param size of the sequence to sort
 * @param seed to use for the random generation of the sequence
 */
void test(int nbThreads, int size, int seed) {
    Quicksort<int> sorter(nbThreads);
    std::vector<int> array = generateSequence(size, seed);
    sorter.sort(array);
    EXPECT_FALSE(array.empty());  // check that the result is not empty
    EXPECT_TRUE(isSorted(array)); // check that result is sorted
}

TEST(SortingTest, NoThreads) {
    // TODO: Modify this test, and add others
    int nbThreads = 0;

    EXPECT_THROW({
         Quicksort<int> sorter(nbThreads);
    }, std::invalid_argument);
}


TEST(SortingTest, Size10Threads1) {
    // TODO: Modify this test, and add others
    int size = 10;
    int nbThreads = 1;
    int seed = 0;

    test(nbThreads, size, seed);
}

TEST(SortingTest, Size1000Threads2) {
    // TODO: Modify this test, and add others
    int size = 1 * 1000;
    int nbThreads = 2;
    int seed = 634;

    test(nbThreads, size, seed);
}

TEST(SortingTest, Size4Thousand8Threads) {
    // TODO: Modify this test, and add others
    int size = 4 * 1000;
    int nbThreads = 8;
    int seed = 12;

    test(nbThreads, size, seed);
}

TEST(SortingTest, Size2Million3Threads) {
    // TODO: Modify this test, and add others
    int size = 2 * 1000 * 1000;
    int nbThreads = 3;
    int seed = 234;

    test(nbThreads, size, seed);
}

TEST(SortingTest, Size10Million16Threads) {
    // TODO: Modify this test, and add others
    int size = 10 * 1000 * 1000;
    int nbThreads = 16;
    int seed = 3;

    test(nbThreads, size, seed);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
