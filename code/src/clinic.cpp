#include "clinic.h"
#include "costs.h"
#include <cassert>
#include <pcosynchro/pcothread.h>
#include <iostream>

IWindowInterface *Clinic::interface = nullptr;

Clinic::Clinic(int uniqueId, int fund, std::vector<ItemType> resourcesNeeded)
    : Seller(fund, uniqueId), nbTreated(0), resourcesNeeded(resourcesNeeded), mutex()
{
    interface->updateFund(uniqueId, fund);
    interface->consoleAppendText(uniqueId, "Clinic created");

    for (const auto &item : resourcesNeeded)
    {
        stocks[item] = 0;
    }
}

bool Clinic::verifyResources()
{
    for (auto item : resourcesNeeded)
    {
        if (stocks[item] == 0)
        {
            return false;
        }
    }
    return true;
}

/**
 * Returns healed patients to hospitals
 */
int Clinic::request(ItemType what, int qty)
{
    assert(what == ItemType::PatientHealed);

    if (qty < 1)
    {
        return 0;
    }

    mutex.lock();

    if (stocks[what] < qty)
    {
        mutex.unlock();
        return 0;
    }

    // We remove the number of patient we transfer from the clinic stock
    stocks[what] -= qty;
    int price = getCostPerUnit(what) * qty;
    money += price;

    mutex.unlock();

    return price;
}

void Clinic::treatPatient()
{
    // TODO

    ItemType item1 = resourcesNeeded[1];
    ItemType item2 = resourcesNeeded[2];

    mutex.lock();

    if (stocks[item1] == 0 ||
        stocks[item2] == 0 ||
        stocks[ItemType::PatientSick] == 0 ||
        money < getEmployeeSalary(getEmployeeThatProduces(ItemType::PatientHealed)))
    {

        mutex.unlock();
        return;
    }

    // We have enough resources, do the thing
    stocks[item1]--;
    stocks[item2]--;

    // Temps simulant un traitement
    interface->simulateWork();

    stocks[ItemType::PatientSick]--;
    stocks[ItemType::PatientHealed]++;

    money -= getEmployeeSalary(getEmployeeThatProduces(ItemType::PatientHealed));
    nbTreated++;

    mutex.unlock();

    interface->consoleAppendText(uniqueId, "Clinic have healed a new patient");
}

void Clinic::orderResources()
{
    const int quantity = 1;

    mutex.lock();

    {
        Seller *hospital = chooseRandomSeller(hospitals);

        // Ensure that we have the funds to buy the resources
        if (money > getCostPerUnit(ItemType::PatientSick) * quantity)
        {
            int price = hospital->request(ItemType::PatientSick, quantity);
            if (price)
            {
                stocks[ItemType::PatientSick] += quantity;
                money -= price;
            }
        }
    }

    // We order one of each needed resources. We do not perform any checks to see which ones we have in stocks and
    // we do not need to order.
    for (auto &supplier : suppliers)
    {
        // We have to skip the first resource as it is ItemType::PatientSick
        for (int i = 1; i < resourcesNeeded.size(); i++)
        {
            auto item = resourcesNeeded[i];
            int estimatedPrice = getCostPerUnit(item) * quantity;
            if (estimatedPrice < money && stocks[item] <= stocks[ItemType::PatientSick] + 1)
            {
                int price = supplier->request(item, quantity);
                if (price)
                {
                    stocks[item] += quantity;
                    money -= price;
                }
            }
        }
    }

    mutex.unlock();
}

void Clinic::run()
{
    if (hospitals.empty() || suppliers.empty())
    {
        std::cerr << "You have to give to hospitals and suppliers to run a clinic" << std::endl;
        return;
    }
    interface->consoleAppendText(uniqueId, "[START] Clinic routine");

    while (!PcoThread::thisThread()->stopRequested())
    {

        if (verifyResources())
        {
            treatPatient();
        }
        else
        {
            orderResources();
        }

        interface->simulateWork();

        interface->updateFund(uniqueId, money);
        interface->updateStock(uniqueId, &stocks);
    }
    interface->consoleAppendText(uniqueId, "[STOP] Clinic routine");
}

void Clinic::setHospitalsAndSuppliers(std::vector<Seller *> hospitals, std::vector<Seller *> suppliers)
{
    this->hospitals = hospitals;
    this->suppliers = suppliers;

    for (Seller *hospital : hospitals)
    {
        interface->setLink(uniqueId, hospital->getUniqueId());
    }
    for (Seller *supplier : suppliers)
    {
        interface->setLink(uniqueId, supplier->getUniqueId());
    }
}

int Clinic::getTreatmentCost()
{
    // TODO??
    return 0;
}

int Clinic::getWaitingPatients()
{
    return stocks[ItemType::PatientSick];
}

int Clinic::getNumberPatients()
{
    return stocks[ItemType::PatientSick] + stocks[ItemType::PatientHealed];
}

int Clinic::send(ItemType it, int qty, int bill)
{
    assert(it == ItemType::PatientSick);

    stocks[ItemType::PatientSick] += qty;

    return qty;
}

int Clinic::getAmountPaidToWorkers()
{
    return nbTreated * getEmployeeSalary(getEmployeeThatProduces(ItemType::PatientHealed));
}

void Clinic::setInterface(IWindowInterface *windowInterface)
{
    interface = windowInterface;
}

std::map<ItemType, int> Clinic::getItemsForSale()
{
    return stocks;
}

Pulmonology::Pulmonology(int uniqueId, int fund) : Clinic::Clinic(uniqueId, fund, {ItemType::PatientSick, ItemType::Pill, ItemType::Thermometer}) {}

Cardiology::Cardiology(int uniqueId, int fund) : Clinic::Clinic(uniqueId, fund, {ItemType::PatientSick, ItemType::Syringe, ItemType::Stethoscope}) {}

Neurology::Neurology(int uniqueId, int fund) : Clinic::Clinic(uniqueId, fund, {ItemType::PatientSick, ItemType::Pill, ItemType::Scalpel}) {}
