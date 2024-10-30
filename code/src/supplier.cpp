#include "supplier.h"
#include "costs.h"
#include <cassert>
#include <pcosynchro/pcothread.h>

IWindowInterface* Supplier::interface = nullptr;

Supplier::Supplier(int uniqueId, int fund, std::vector<ItemType> resourcesSupplied)
    : Seller(fund, uniqueId), resourcesSupplied(resourcesSupplied), nbSupplied(0), mutex()
{
    for (const auto& item : resourcesSupplied) {    
        stocks[item] = 0;    
    }

    interface->consoleAppendText(uniqueId, QString("Supplier Created"));
    interface->updateFund(uniqueId, fund);
}


int Supplier::request(ItemType it, int qty) {
    // TODO
    assert(it != ItemType::Syringe || it != ItemType::Pill || it != ItemType::Scalpel || it != ItemType::Thermometer || it != ItemType::Stethoscope);

    if (qty < 1) {
        return 0;
    }

    if (stocks[it] < qty) {
        // We do not have enough items in stock.
        return 0;
    }

    mutex.lock();

    stocks[it] -= qty;
    int price = qty * getCostPerUnit(it);
    money += price;

    mutex.unlock();

    return price;
}

void Supplier::run() {
    interface->consoleAppendText(uniqueId, "[START] Supplier routine");
    while (!PcoThread::thisThread()->stopRequested()) {
        ItemType resourceSupplied = getRandomItemFromStock();
        int supplierCost = getEmployeeSalary(getEmployeeThatProduces(resourceSupplied));
        // TODO
        if (money >= supplierCost) {
            stocks[resourceSupplied]++;
            money -= supplierCost;
        }

        /* Temps aléatoire borné qui simule l'attente du travail fini*/
        interface->simulateWork();
        //TODO

        nbSupplied++;

        interface->updateFund(uniqueId, money);
        interface->updateStock(uniqueId, &stocks);
    }
    interface->consoleAppendText(uniqueId, "[STOP] Supplier routine");
}


std::map<ItemType, int> Supplier::getItemsForSale() {
    return stocks;
}

int Supplier::getMaterialCost() {
    int totalCost = 0;
    for (const auto& item : resourcesSupplied) {
        totalCost += getCostPerUnit(item);
    }
    return totalCost;
}

int Supplier::getAmountPaidToWorkers() {
    return nbSupplied * getEmployeeSalary(EmployeeType::Supplier);
}

void Supplier::setInterface(IWindowInterface *windowInterface) {
    interface = windowInterface;
}

std::vector<ItemType> Supplier::getResourcesSupplied() const
{
    return resourcesSupplied;
}

int Supplier::send(ItemType it, int qty, int bill){
    return 0;
}
