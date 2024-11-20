#ifndef SHARED_STATION_H
#define SHARED_STATION_H

#include <pcosynchro/pcosemaphore.h>
#include "locomotive.h"
#include <map>

class SharedStation
{
public:
    SharedStation(int nbTrains, int nbTours);

    void enterStation(Locomotive &loco);

private:
    int nbTrains;
    int nbTours;
    int currentlyWaiting;

    PcoSemaphore stationWait{0};
    PcoSemaphore lock{1};

    std::map<int, int> currentTurnCount;
};

#endif // SHARED_STATION_H
