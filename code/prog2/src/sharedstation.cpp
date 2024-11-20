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
            for (auto& [_, v] : currentTurnCount) v = 0;

            for (int i = 1; i < nbTrains; ++i)
            {
                stationWait.release();
            }

            // TODO(alexandre): I don't know if this is the best way to do this but I can't think of a better place
            // What we want to do is toggle the priority mode once the trains turn around. But we cannot do that inside
            // the `locomotivebehavior` as we only want to call this once per cycle.
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
