#include "hospital.h"
#include "costs.h"
#include <iostream>
#include <cassert>
#include <pcosynchro/pcothread.h>

IWindowInterface* Hospital::interface = nullptr;

Hospital::Hospital(int uniqueId, int fund, int maxBeds)
    : Seller(fund, uniqueId), maxBeds(maxBeds), currentBeds(0), nbHospitalised(0), nbFree(0), mutex()
{
    interface->updateFund(uniqueId, fund);
    interface->consoleAppendText(uniqueId, "Hospital Created with " + QString::number(maxBeds) + " beds");

    std::vector<ItemType> initialStocks = { ItemType::PatientHealed, ItemType::PatientSick };

    for(const auto& item : initialStocks) {
        stocks[item] = 0;
    }
}


/**
 * Transfers patient out of the hospital (into clinics)
 */
int Hospital::request(ItemType what, int qty) {
    // The method documentation talks about either requesting sick or healed patient.
    assert(what == ItemType::PatientSick || what == ItemType::PatientHealed);

    if (qty < 1) {
        return 0;
    }

    if (stocks[what] < qty) {
        return 0;
    }

    mutex.lock();

    stocks[ItemType::PatientSick] -= qty;
    currentBeds -= qty;

    int price = getCostPerUnit(what) * qty;
    money += price;

    mutex.unlock();

    return price;
}

void Hospital::freeHealedPatient() {
    // TODO
    mutex.lock();

    int toFree = patientsToFree.front();
    nbFree += toFree;
    currentBeds -= toFree;
    stocks[ItemType::PatientHealed] -= toFree;

    /**
     * We shift the position of patients to free to the left as we handled the release of patient
     * for the day.
     */
    for (size_t i = patientsToFree.size() -1; i > 0; i--) {
        patientsToFree[i - 1] = patientsToFree[i];
    }
    patientsToFree.back() = 0;

    // As discussed, the hospital gets money each time a patient is transferred out of the hospital.
    money += getCostPerUnit(ItemType::PatientHealed) * toFree;

    mutex.unlock();
}

/**
 * Gets patients from clinic back into the hospital
 */
void Hospital::transferPatientsFromClinic() {
    const int quantity = 1;

    // TODO
    Seller* clinic = chooseRandomSeller(clinics);

    if (currentBeds >= maxBeds) {
        // We cannot accept any healed patients as we don't have any free beds.
        return;
    }

    // If we do not have the funds available to get healed patients from clinic we do not transfer them.
    int price = getCostPerUnit(ItemType::PatientHealed) * quantity;
    if (money < price) {
        return;
    }

    mutex.lock();

    int debit = clinic->request(ItemType::PatientHealed, quantity);
    if (debit) {
        stocks[ItemType::PatientHealed] += quantity;
        currentBeds += quantity;
        nbHospitalised += quantity;

        patientsToFree.at(patientsToFree.size() - 1) = quantity;

        money -= debit;
    }

    mutex.unlock();
}

/**
 *
 */
int Hospital::send(ItemType it, int qty, int bill) {
    // TODO
    assert(it == ItemType::PatientSick);

    mutex.lock();

    int price = getCostPerUnit(it) * qty;
    if (money < price) {
        return 0;
    }

    int availableBeds = maxBeds - currentBeds;
    int takes = std::min(qty, availableBeds);
    stocks[ItemType::PatientSick] += takes;
    currentBeds += takes;
    nbHospitalised += takes;

    money -= price;

    mutex.unlock();

    return price;
}

void Hospital::run()
{
    if (clinics.empty()) {
        std::cerr << "You have to give clinics to a hospital before launching is routine" << std::endl;
        return;
    }

    interface->consoleAppendText(uniqueId, "[START] Hospital routine");

    while (!PcoThread::thisThread()->stopRequested()) {
        transferPatientsFromClinic();

        freeHealedPatient();

        interface->updateFund(uniqueId, money);
        interface->updateStock(uniqueId, &stocks);
        interface->simulateWork(); // Temps d'attente
    }

    interface->consoleAppendText(uniqueId, "[STOP] Hospital routine");
}

int Hospital::getAmountPaidToWorkers() {
    return nbHospitalised * getEmployeeSalary(EmployeeType::Nurse);
}

int Hospital::getNumberPatients(){
    return stocks[ItemType::PatientSick] + stocks[ItemType::PatientHealed] + nbFree;
}

std::map<ItemType, int> Hospital::getItemsForSale()
{
    return stocks;
}

void Hospital::setClinics(std::vector<Seller*> clinics){
    this->clinics = clinics;

    for (Seller* clinic : clinics) {
        interface->setLink(uniqueId, clinic->getUniqueId());
    }
}

void Hospital::setInterface(IWindowInterface* windowInterface){
    interface = windowInterface;
}
