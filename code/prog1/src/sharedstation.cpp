//    ___  _________    ___  ___  ___ ____ //
//   / _ \/ ___/ __ \  |_  |/ _ \|_  / / / //
//  / ___/ /__/ /_/ / / __// // / __/_  _/ //
// /_/   \___/\____/ /____/\___/____//_/   //
//

#include <chrono>
#include <thread>

#include "sharedstation.h"

#include <pcosynchro/pcothread.h>

SharedStation::SharedStation(int nbTrains, int nbTours): nbTrains(nbTrains), nbTours(nbTours)
{
}


void SharedStation::enterStation(Locomotive& loco)
{
    lock.acquire();
    int turnCount;
    try
    {
        turnCount = currentTurnCount.at(loco.numero());
    }
    catch ([[maybe_unused]] const std::out_of_range& ex)
    {
        turnCount = 0;
    }

    currentTurnCount[loco.numero()] = ++turnCount;


    if (turnCount == nbTours)
    {
        loco.arreter();
        if (currentlyWaiting + 1 == nbTrains)
        {
            loco.afficherMessage("Attente, puis liberation");
            PcoThread::usleep(2 * 1000 * 1000);
            currentlyWaiting = 0;
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
