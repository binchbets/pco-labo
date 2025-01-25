#ifndef MODELBRIDGEMANAGER_H
#define MODELBRIDGEMANAGER_H

#include <iostream>
#include <utility>

#include "pcomodel.h"
#include "pcosynchro/pcosemaphore.h"
#include "pcoconcurrencyanalyzer.h"


#define EDGE(first, second) first->next.push_back(second)

class BridgeManager
{
    PcoSemaphore mutex;
    PcoSemaphore waitingAccess;
    int nbWaitingAccess;
    float currentWeight;
    float maxWeight;

public:
    BridgeManager(float maxWeight) : mutex(1), waitingAccess(0)
    {
        this->maxWeight = maxWeight;
        currentWeight = 0;
        nbWaitingAccess = 0;
    }

    void access(float weight)
    {
        startSection(1);

        mutex.acquire();

        while (currentWeight + weight > maxWeight) {
            startSection(2);
            nbWaitingAccess++;
            mutex.release();
            waitingAccess.acquire();
            mutex.acquire();
        }

        //startSection(3);
        currentWeight += weight;

        if (nbWaitingAccess > 0) {
            startSection(4);
            nbWaitingAccess--;
            waitingAccess.release();
        }
        startSection(5);

        mutex.release();

    }

    void leave(float weight)
    {
        mutex.acquire();

        //startSection(6);
        currentWeight -= weight;

        if (nbWaitingAccess > 0) {
            startSection(7);
            nbWaitingAccess--;
            waitingAccess.release();
        }

        startSection(8);
        mutex.release();
    }
};


class ThreadVehicle : public ObservableThread
{
private:
    BridgeManager& bridgeManager;
    float weight;

public:
    explicit ThreadVehicle(BridgeManager &bridgeManager, std::string id = "", float weight = 10.0f) : bridgeManager(bridgeManager), ObservableThread(std::move(id)), weight(weight)
    {
        scenarioGraph = std::make_unique<ScenarioGraph>();
        auto scenario = scenarioGraph->createNode(this, -1);
        auto p1 = scenarioGraph->createNode(this, 1);
        auto p2 = scenarioGraph->createNode(this, 2);
        //auto p3 = scenarioGraph->createNode(this, 3);
        auto p4 = scenarioGraph->createNode(this, 4);
        auto p5 = scenarioGraph->createNode(this, 5);
        //auto p6 = scenarioGraph->createNode(this, 6);
        auto p7 = scenarioGraph->createNode(this, 7);
        auto p8 = scenarioGraph->createNode(this, 8);
        scenario->next.push_back(p1);

        EDGE(scenario, p1);

        EDGE(p1, p2);
        EDGE(p1, p4);
        EDGE(p1, p5);

        //EDGE(p1, p3);

        EDGE(p2, p2);
        EDGE(p2, p4);
        EDGE(p2, p5);

        //EDGE(p3, p4);
        //EDGE(p3, p5);

        EDGE(p4, p5);

        EDGE(p5, p7);
        EDGE(p5, p8);

        //EDGE(p6, p7);
        //EDGE(p6, p8);

        EDGE(p7, p8);

        scenarioGraph->setInitialNode(scenario);
    }

private:
    void run() override
    {
        bridgeManager.access(weight);
        bridgeManager.leave(weight);
        endScenario();
    }
};

class ModelBridgeManager : public PcoModel
{
private:
    BridgeManager bridgeManager;

public:
    ModelBridgeManager(): bridgeManager(60.0f) {}

    bool checkInvariants() override 
    {
        return true;
    }
    
    
    void build() override
    {
        threads.emplace_back(std::make_unique<ThreadVehicle>(bridgeManager, "car", 3.0f));
        threads.emplace_back(std::make_unique<ThreadVehicle>(bridgeManager, "truck", 31.0f));

        scenarioBuilder = std::make_unique<ScenarioBuilderBuffer>();
        scenarioBuilder->init(threads, 13);
    }

    void preRun(Scenario& /*scenario*/) override
    {
    }

    void postRun(Scenario &scenario) override
    {
        /* Commented out for performance.
        std::cout << "---------------------------------------" << std::endl;
        std::cout << "Scenario : ";
        ScenarioPrint::printScenario(scenario);
         */
    }

    void finalReport() override 
    {
        std::cout << "---------------------------------------" << std::endl;
        std::cout << "Possible output number : ";
        std::cout << std::endl;
        std::cout << std::flush;
    }
};


#endif // MODELBRIDGEMANAGER_H