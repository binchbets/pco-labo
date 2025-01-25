#ifndef MODELBRIDGEMANAGER_H
#define MODELBRIDGEMANAGER_H

#include <iostream>
#include <utility>

#include "pcomodel.h"
#include "pcosynchro/pcosemaphore.h"
#include "pcoconcurrencyanalyzer.h"

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

        currentWeight += weight;

        if (nbWaitingAccess > 0) {
            nbWaitingAccess--;
            waitingAccess.release();
        }
        startSection(3);

        mutex.release();

    }

    void leave(float weight)
    {
        mutex.acquire();
        currentWeight -= weight;
        if (nbWaitingAccess > 0) {
            nbWaitingAccess--;
            waitingAccess.release();
        }

        mutex.release();
    }
};


class ThreadCar : public ObservableThread
{
private:
    BridgeManager& bridgeManager;
    float WEIGHT = 3.0f;

public:
    explicit ThreadCar(BridgeManager &bridgeManager, std::string id = "") : bridgeManager(bridgeManager), ObservableThread(std::move(id))
    {
        scenarioGraph = std::make_unique<ScenarioGraph>();
        auto scenario = scenarioGraph->createNode(this, -1);
        auto p1 = scenarioGraph->createNode(this, 1);
        auto p2 = scenarioGraph->createNode(this, 2);
        auto p3 = scenarioGraph->createNode(this, 3);
        scenario->next.push_back(p1);
        p1->next.push_back(p2);
        p1->next.push_back(p3);
        p2->next.push_back(p2);
        p2->next.push_back(p3);
        scenarioGraph->setInitialNode(scenario);
    }

private:
    void run() override
    {
        bridgeManager.access(WEIGHT);
        bridgeManager.leave(WEIGHT);
        endScenario();
    }
};

class ThreadTruck : public ObservableThread
{
private:
    BridgeManager& bridgeManager;
    float WEIGHT = 30.0f;

public:
    explicit ThreadTruck(BridgeManager &bridgeManager, std::string id = "") : bridgeManager(bridgeManager), ObservableThread(std::move(id))
    {
        scenarioGraph = std::make_unique<ScenarioGraph>();
        auto scenario = scenarioGraph->createNode(this, -1);
        auto p1 = scenarioGraph->createNode(this, 1);
        auto p2 = scenarioGraph->createNode(this, 2);
        auto p3 = scenarioGraph->createNode(this, 3);
        scenario->next.push_back(p1);
        p1->next.push_back(p2);
        p1->next.push_back(p3);
        p2->next.push_back(p2);
        p2->next.push_back(p3);
        scenarioGraph->setInitialNode(scenario);
    }

private:
    void run() override
    {
        bridgeManager.access(WEIGHT);
        startSection(2);
        bridgeManager.leave(WEIGHT);
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
        threads.emplace_back(std::make_unique<ThreadCar>(bridgeManager, "car"));
        threads.emplace_back(std::make_unique<ThreadTruck>(bridgeManager, "truck1"));
        threads.emplace_back(std::make_unique<ThreadTruck>(bridgeManager, "truck2"));

        scenarioBuilder = std::make_unique<ScenarioBuilderBuffer>();
        scenarioBuilder->init(threads, 9);
    }

    void preRun(Scenario& /*scenario*/) override
    {
    }

    void postRun(Scenario &scenario) override
    {
        std::cout << "---------------------------------------" << std::endl;
        std::cout << "Scenario : ";
        ScenarioPrint::printScenario(scenario);
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