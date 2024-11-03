#include "hospital.h"
#include "costs.h"
#include <iostream>
#include <cassert>
#include <pcosynchro/pcothread.h>

IWindowInterface *Hospital::interface = nullptr;

Hospital::Hospital(int uniqueId, int fund, int maxBeds)
    : Seller(fund, uniqueId), maxBeds(maxBeds), currentBeds(0), nbHospitalised(0), nbFree(0), mutex()
{
    interface->updateFund(uniqueId, fund);
    interface->consoleAppendText(uniqueId, "Hospital Created with " + QString::number(maxBeds) + " beds");

    std::vector<ItemType> initialStocks = {ItemType::PatientHealed, ItemType::PatientSick};

    for (const auto &item : initialStocks)
    {
        stocks[item] = 0;
    }
}

/**
 * Transfers patient out of the hospital (into clinics)
 */
int Hospital::request(ItemType what, int qty)
{
    // The method documentation talks about either requesting sick or healed patient.
    assert(what == ItemType::PatientSick);

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

    stocks[ItemType::PatientSick] -= qty;
    currentBeds -= qty;

    int price = getCostPerUnit(what) * qty;
    money += price;

    mutex.unlock();

    return price;
}

void Hospital::freeHealedPatient()
{
    mutex.lock();

    int toFree = patientsToFree.front();
    nbFree += toFree;
    currentBeds -= toFree;
    stocks[ItemType::PatientHealed] -= toFree;

    /**
     * We shift the position of patients to free to the left as we handled the release of patient
     * for the day.
     */
    for (size_t i = patientsToFree.size() - 1; i > 0; i--)
    {
        patientsToFree[i - 1] = patientsToFree[i];
    }
    patientsToFree.back() = 0;

    mutex.unlock();
}

/**
 * Gets patients from clinic back into the hospital
 */
void Hospital::transferPatientsFromClinic()
{
    const int quantity = 1;

    // TODO
    Seller *clinic = chooseRandomSeller(clinics);

    // We have to lock before the price check as we might receive patients from an ambulance at the same time we call
    // this method, which would invalidate our funds validation as money has been deduced to pay nurses when a patient
    // has been transferred from an ambulance.
    mutex.lock();

    if (currentBeds >= maxBeds)
    {
        // We cannot accept any healed patients as we don't have any free beds.
        mutex.unlock();
        return;
    }

    // If we do not have the funds available to get healed patients from clinic we do not transfer them.
    // We also have to take into account the nurse's salary.
    int price = getCostPerUnit(ItemType::PatientHealed) * quantity;
    price += getEmployeeSalary(EmployeeType::Nurse);
    if (money < price)
    {
        mutex.unlock();
        return;
    }


    int debit = clinic->request(ItemType::PatientHealed, quantity);
    if (debit)
    {
        stocks[ItemType::PatientHealed] += quantity;
        currentBeds += quantity;
        nbHospitalised += quantity;

        patientsToFree.at(patientsToFree.size() - 1) = quantity;

        money -= debit;
        money -= getEmployeeSalary(EmployeeType::Nurse);
    }

    mutex.unlock();
}

/**
 *
 */
int Hospital::send(ItemType it, int qty, int bill)
{
    assert(it == ItemType::PatientSick);

    mutex.lock();

    int price = getCostPerUnit(it) * qty;
    price += getEmployeeSalary(EmployeeType::Nurse);
    int availableBeds = maxBeds - currentBeds;
    if (money < price || availableBeds < qty)
    {
        mutex.unlock();
        return 0;
    }

    interface->consoleAppendText(uniqueId, "Hospital has received " + QString::number(qty) + " patients");

    stocks[ItemType::PatientSick] += qty;
    currentBeds += qty;
    nbHospitalised += qty;

    money -= price;

    mutex.unlock();

    return getCostPerUnit(it) * qty;
}

void Hospital::run()
{
    if (clinics.empty())
    {
        std::cerr << "You have to give clinics to a hospital before launching is routine" << std::endl;
        return;
    }

    interface->consoleAppendText(uniqueId, "[START] Hospital routine");

    while (!PcoThread::thisThread()->stopRequested())
    {
        transferPatientsFromClinic();

        freeHealedPatient();

        interface->updateFund(uniqueId, money);
        interface->updateStock(uniqueId, &stocks);
        interface->simulateWork(); // Temps d'attente
    }

    interface->consoleAppendText(uniqueId, "[STOP] Hospital routine");
}

int Hospital::getAmountPaidToWorkers()
{
    return nbHospitalised * getEmployeeSalary(EmployeeType::Nurse);
}

int Hospital::getNumberPatients()
{
    return stocks[ItemType::PatientSick] + stocks[ItemType::PatientHealed] + nbFree;
}

std::map<ItemType, int> Hospital::getItemsForSale()
{
    return stocks;
}

void Hospital::setClinics(std::vector<Seller *> clinics)
{
    this->clinics = clinics;

    for (Seller *clinic : clinics)
    {
        interface->setLink(uniqueId, clinic->getUniqueId());
    }
}

void Hospital::setInterface(IWindowInterface *windowInterface)
{
    interface = windowInterface;
}
