
#include "mythread.h"
#include <iostream>
#include <QDebug>

bool isSorted(std::vector<int> seq)
{
    for (size_t i = 0; i < seq.size(); ++i)
    {
        if (i >= 1 && seq[i - 1] > seq[i])
        {
            return false;
        }
    }
    return true;
}

std::vector<int> getPermutation(std::vector<int> seq, int k)
{
    size_t n = seq.size();
    std::vector<int> permutation(n);
    std::vector<int> availableElements = seq;

    for (size_t i = 0; i < n; ++i)
    {
        long fact = factorial(n - 1 - i);
        size_t idx = k / fact;
        std::vector<int>::iterator iter = availableElements.begin() + idx;
        permutation[i] = *iter;
        availableElements.erase(iter);

        k = k % fact;
    }

    return permutation;
}

void bogosort(std::vector<int> seq, ThreadManager *pManager, size_t startSeed, size_t count, std::vector<int> &result)
{
    qDebug("Started thread handling from %li to %li", startSeed, startSeed + count);
    int totalCount = factorial(seq.size());
    /*
    Get permutation. If it was right, return it
    */
    for (size_t k = startSeed; k < startSeed + count; ++k)
    {
        if (pManager->finished || PcoThread::thisThread()->stopRequested())
        {
            return;
        }

        std::vector<int> attempt = getPermutation(seq, k);

        if (k % 100 == 0)
        {
            pManager->incrementPercentComputed((double)100 / totalCount);
        }

        if (isSorted(attempt))
        {
            result = attempt;
            pManager->finished = true;
            return;
        }
    }
}
