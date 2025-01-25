#ifndef MODELBRIDGEMANAGER_H
#define MODELBRIDGEMANAGER_H

#include <iostream>

#include "pcomodel.h"
#include "pcosynchro/pcosemaphore.h"
#include "pcoconcurrencyanalyzer.h"

class ThreadCar : public ObservableThread
{
public:
    explicit ThreadCar(std::string id = "") : ObservableThread(std::move(id))
    {
        scenarioGraph = std::make_unique<ScenarioGraph>();
        auto scenario = scenarioGraph->createNode(this, -1);
        auto p1 = scenarioGraph->createNode(this, 1);
        auto p2 = scenarioGraph->createNode(this, 2);
        auto p3 = scenarioGraph->createNode(this, 3);
        scenario->next.push_back(p1);
        p1->next.push_back(p2);
        p2->next.push_back(p3);
        scenarioGraph->setInitialNode(scenario);
    }

private:
    static std::unique_ptr<PcoSemaphore> semaphore;
    void run() override
    {
        endScenario();
    }
};

class ThreadTruck : public ObservableThread
{
public:
    explicit ThreadTruck(std::string id = "") : ObservableThread(std::move(id))
    {
        scenarioGraph = std::make_unique<ScenarioGraph>();
        auto scenario = scenarioGraph->createNode(this, -1);
        auto p1 = scenarioGraph->createNode(this, 4);
        auto p2 = scenarioGraph->createNode(this, 5);
        auto p3 = scenarioGraph->createNode(this, 6);
        scenario->next.push_back(p1);
        p1->next.push_back(p2);
        p2->next.push_back(p3);
        scenarioGraph->setInitialNode(scenario);
    }

private:
    static std::unique_ptr<PcoSemaphore> semaphore;
    void run() override
    {
        endScenario();
    }
};

class ModelBridgeManager : public PcoModel
{
    bool checkInvariants() override 
    {
        return true;
    }
    
    
    void build() override
    {
        threads.emplace_back(std::make_unique<ThreadCar>("car"));
        threads.emplace_back(std::make_unique<ThreadTruck>("truck"));

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

#endif // MODELBRIDGEMANAGER_H