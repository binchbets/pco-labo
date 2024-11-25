//    ___  _________    ___  ___  ___ ____ //
//   / _ \/ ___/ __ \  |_  |/ _ \|_  / / / //
//  / ___/ /__/ /_/ / / __// // / __/_  _/ //
// /_/   \___/\____/ /____/\___/____//_/   //
//

#include <chrono>
#include <thread>

#include "sharedstation.h"
#include "sharedsection.h"

#include <pcosynchro/pcothread.h>

SharedStation::SharedStation(int nbTrains, int nbTours, std::shared_ptr<SharedSectionInterface> sharedSection): nbTrains(nbTrains), nbTours(nbTours), currentlyWaiting(0), sharedSection(sharedSection)
{
}


void SharedStation::enterStation(Locomotive& loco)
{
    lock.acquire();
    int turnCount = ++currentTurnCount[loco.numero()];

    if (turnCount == nbTours)
    {
        loco.arreter();
        if (currentlyWaiting + 1 == nbTrains)
        {
            loco.afficherMessage("Attente, puis liberation");
            PcoThread::usleep(2 * 1000 * 1000);
            currentlyWaiting = 0;

            // Reset all turn counts
            currentTurnCount.clear();

            for (int i = 1; i < nbTrains; ++i)
            {
                stationWait.release();
            }

            // We reset the priority mode every time we stop at the station, as asked in the specifications.
            sharedSection->togglePriorityMode();

            lock.release();
        } else
        {
            loco.afficherMessage("arrêt");
            currentlyWaiting++;
            lock.release();
            // Block
            stationWait.acquire();
        }

        // Setup random number generator
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(1, 10);

        // We set a new priority to the train
        loco.priority = dist(gen);

        loco.inverserSens();
        loco.demarrer();
    }
    else
    {
        loco.afficherMessage("pas bon tour.");
        lock.release();
    }
}
