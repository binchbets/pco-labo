
#include <pcosynchro/pcoconditionvariable.h>
#include <pcosynchro/pcohoaremonitor.h>
#include <pcosynchro/pcomutex.h>
#include <pcosynchro/pcosemaphore.h>


class BridgeManagerFloat
{
public:
    virtual void access(float weight) = 0;
    virtual void leave(float weight) = 0;
};

class BridgeManager : public BridgeManagerFloat
{
    PcoSemaphore mutex;
    PcoSemaphore waitingAccess;
    PcoSemaphore fifo;
    int nbWaitingAccess;
    float currentWeight;
    float maxWeight;
    float waitingWeight;

public:
    BridgeManager(float maxWeight) : mutex(1), waitingAccess(0), fifo(1)
    {
        this->maxWeight = maxWeight;
        currentWeight = 0.0;
        nbWaitingAccess = 0;
        waitingWeight = 0.0;
    }

    void access(float weight) override
    {
        fifo.acquire();
        mutex.acquire();

        // Because of the if, it is wrong
        if (currentWeight + weight > maxWeight) {
            waitingWeight = currentWeight;
            mutex.release();
            waitingAccess.acquire();
        }

        currentWeight += weight;
        mutex.release();
        fifo.release();
    }

    void leave(float weight) override
    {
        mutex.acquire();
        currentWeight -= weight;
        if ((waitingWeight > 0.0) && (currentWeight + waitingWeight <= maxWeight)) {
            waitingWeight = 0.0;
            waitingAccess.release();
        }
        else {
            mutex.release();
        }
    }
};