#include "hospital.h"
#include "costs.h"
#include <iostream>
#include <cassert>
#include <pcosynchro/pcothread.h>

IWindowInterface* Hospital::interface = nullptr;

Hospital::Hospital(int uniqueId, int fund, int maxBeds)
    : Seller(fund, uniqueId), maxBeds(maxBeds), currentBeds(0), nbHospitalised(0), nbFree(0)
{
    interface->updateFund(uniqueId, fund);
    interface->consoleAppendText(uniqueId, "Hospital Created with " + QString::number(maxBeds) + " beds");

    std::vector<ItemType> initialStocks = { ItemType::PatientHealed, ItemType::PatientSick };

    for(const auto& item : initialStocks) {
        stocks[item] = 0;
    }
}

int Hospital::request(ItemType what, int qty) {
    // The method documentation talks about either requesting sick or healed patient.
    assert(what == ItemType::PatientSick || what == ItemType::PatientHealed);

    int currentSickPatients = nbHospitalised - nbFree;
    int takes =  std::min(currentSickPatients, qty);
    nbHospitalised -= takes;

    return takes;
}

void Hospital::freeHealedPatient() {
    // TODO
    int toFree = patientsToFree.front();
    nbFree += toFree;
    currentBeds -= toFree;

    /**
     * We shift the position of patients to free to the left as we handled the release of patient
     * for the day.
     */
    for (size_t i = patientsToFree.size() -1; i > 0; i--) {
        patientsToFree[i - 1] = patientsToFree[i];
    }
    patientsToFree.back() = 0;
}

void Hospital::transferPatientsFromClinic() {
    // TODO
    Seller* clinic = chooseRandomSeller(clinics);

    if (currentBeds >= maxBeds) {
        // We cannot accept any healed patients as we don't have any free beds.
        return;
    }

    int quantity = 1;
    if (clinic->request(ItemType::PatientHealed, quantity)) {
        stocks[ItemType::PatientHealed] += quantity;
        currentBeds += quantity;
        nbHospitalised += quantity;

        patientsToFree.at(patientsToFree.size() - 1) = quantity;
    }
}

// TODO: Should we return the number of patients we took
//       even if this number is less than the `qty` or
//       should we just refuse the transfer ?
int Hospital::send(ItemType it, int qty, int bill) {
    // TODO
    assert(it == ItemType::PatientSick);

    int availableBeds = maxBeds - currentBeds;
    int takes = std::min(qty, availableBeds);
    stocks[ItemType::PatientSick] += takes;
    currentBeds += takes;
    nbHospitalised += takes;

    return takes;
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
