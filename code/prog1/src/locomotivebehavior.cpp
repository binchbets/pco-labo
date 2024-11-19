//    ___  _________    ___  ___  ___ ____ //
//   / _ \/ ___/ __ \  |_  |/ _ \|_  / / / //
//  / ___/ /__/ /_/ / / __// // / __/_  _/ //
// /_/   \___/\____/ /____/\___/____//_/   //
//

#include "locomotivebehavior.h"
#include "ctrain_handler.h"

PcoSemaphore LocomotiveBehavior::releaseAtStation = PcoSemaphore(0);
PcoSemaphore LocomotiveBehavior::waitAtStationMutex = PcoSemaphore(1);
bool LocomotiveBehavior::waitAtStation = true;

void LocomotiveBehavior::run()
{
    //Initialisation de la locomotive
    loco.allumerPhares();
    loco.demarrer();
    loco.afficherMessage("Ready!");

    while (!PcoThread::thisThread()->stopRequested())
    {
        attendre_contact(beforeSharedSectionContactId);
        sharedSection->access(loco);

        for (auto [switchNumber, switchDirection] : switchSetups)
        {
            diriger_aiguillage(switchNumber, switchDirection, 0);
        }

        attendre_contact(afterSharedSectionContactId);
        sharedSection->leave(loco);

        attendre_contact(stationContactId);

        sharedStation->enterStation(loco);

        std::swap(beforeSharedSectionContactId, afterSharedSectionContactId);

        loco.afficherMessage(QString::fromStdString("J'ai atteint le contact " + std::to_string(stationContactId)));
    }
}

void LocomotiveBehavior::printStartMessage()
{
    qDebug() << "[START] Thread de la loco" << loco.numero() << "lancé";
    loco.afficherMessage("Je suis lancée !");
}

void LocomotiveBehavior::printCompletionMessage()
{
    qDebug() << "[STOP] Thread de la loco" << loco.numero() << "a terminé correctement";
    loco.afficherMessage("J'ai terminé");
}
