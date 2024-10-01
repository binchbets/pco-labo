
#include <QRandomGenerator>
#include <iostream>

#include "threadmanager.h"
#include "mythread.h"

ThreadManager::ThreadManager(QObject *parent) : QObject(parent)
{
}

/**
 * @brief Cette fonction trie une séquence générée aléatoirement
 * @param seq séquence à trier
 * @param nbThreads le nombre de threads à lancer
 * @return séquence triée
 */
std::vector<int> ThreadManager::startSorting(
    std::vector<int> seq,
    unsigned int nbThreads)
{
    finished = false;

    unsigned int maxTasks = factorial(seq.size());

    // We cap the number of threads to the max number of available of tasks
    // -> This is to ensure that only one thread can find the sorted sequence.
    nbThreads = std::max(maxTasks, nbThreads);

    long sizePerThread = (maxTasks / nbThreads) + 1;

    std::vector<int> result = seq;

    std::vector<PcoThread *> threads;

    for (unsigned int i = 0; i < nbThreads; ++i)
    {
        threads.push_back(new PcoThread([=, &result]
                                        { bogosort(seq, this, sizePerThread * i, sizePerThread, result); }));
    }

    for (PcoThread *thread : threads)
    {
        thread->join();
        delete thread;
    }

    return result;
}

void ThreadManager::incrementPercentComputed(double percentComputed)
{
    emit sig_incrementPercentComputed(percentComputed);
}
