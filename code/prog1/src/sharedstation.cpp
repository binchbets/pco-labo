//    ___  _________    ___  ___  ___ ____ //
//   / _ \/ ___/ __ \  |_  |/ _ \|_  / / / //
//  / ___/ /__/ /_/ / / __// // / __/_  _/ //
// /_/   \___/\____/ /____/\___/____//_/   //
//

#include "sharedstation.h"

#include <pcosynchro/pcothread.h>

SharedStation::SharedStation(int nbTrains, int nbTours): nbTrains(nbTrains), nbTours(nbTours), currentlyWaiting(0)
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
            lock.release();
        } else
        {
            loco.afficherMessage("arrêt");
            currentlyWaiting++;
            lock.release();
            // Block
            stationWait.acquire();
        }

        loco.inverserSens();
        loco.demarrer();
    }
    else
    {
        loco.afficherMessage("pas bon tour.");
        lock.release();
    }
}
