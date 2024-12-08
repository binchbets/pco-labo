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
    int nbThreads = 0;

    EXPECT_THROW({
         Quicksort<int> sorter(nbThreads);
    }, std::invalid_argument);
}


TEST(SortingTest, Size10Threads1) {
    int size = 10;
    int nbThreads = 1;
    int seed = 0;

    test(nbThreads, size, seed);
}

TEST(SortingTest, Size1000Threads2) {
    int size = 1 * 1000;
    int nbThreads = 2;
    int seed = 634;

    test(nbThreads, size, seed);
}

TEST(SortingTest, Size4Thousand8Threads) {
    int size = 4 * 1000;
    int nbThreads = 8;
    int seed = 12;

    test(nbThreads, size, seed);
}

TEST(SortingTest, Size2Million3Threads) {
    int size = 2 * 1000 * 1000;
    int nbThreads = 3;
    int seed = 234;

    test(nbThreads, size, seed);
}

TEST(SortingTest, Size10Million16Threads) {
    int size = 10 * 1000 * 1000;
    int nbThreads = 16;
    int seed = 3;

    test(nbThreads, size, seed);
}

TEST(SortingTest, ConcurrencyTest) {
    // TODO: Modify this test, and add others
    int size = 1 * 1000;
    int nbThreads = 2;
    int seed = 634;

    for (int i = 0; i < 10000; ++i) {
        test(nbThreads, size, seed);
        std::cout << "Ran iteration " << i << std::endl;
    }
}

TEST(SortingTest, SortedSameValuesTest) {
    int size = 10 * 1000 * 1000;
    int nbThreads = 4;
    int seed = 3;

    Quicksort<int> sorter(nbThreads);
    std::vector<int> array = generateSequence(size, seed);
    std::vector<int> copy(array);
    std::sort(copy.begin(), copy.end());

    sorter.sort(array);

    EXPECT_TRUE(isSorted(array));
    EXPECT_TRUE(std::equal(copy.begin(), copy.end(), array.begin()));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
