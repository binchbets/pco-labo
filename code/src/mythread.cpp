
#include "mythread.h"
#include <iostream>

void bogosort(std::vector<int> seq, ThreadManager *pManager)
{

    // Exemple de mise à jour de la barre de progression
    pManager->incrementPercentComputed((double)1);
}
