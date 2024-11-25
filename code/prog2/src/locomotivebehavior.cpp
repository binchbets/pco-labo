//    ___  _________    ___  ___  ___ ____ //
//   / _ \/ ___/ __ \  |_  |/ _ \|_  / / / //
//  / ___/ /__/ /_/ / / __// // / __/_  _/ //
// /_/   \___/\____/ /____/\___/____//_/   //
//

#include "locomotivebehavior.h"
#include "ctrain_handler.h"
#include "sharedstation.h"
#include <algorithm>
#include <iostream>

void LocomotiveBehavior::run()
{
    //Initialisation de la locomotive
    loco.allumerPhares();
    loco.demarrer();
    loco.afficherMessage("Ready!");

    while (!PcoThread::thisThread()->stopRequested())
    {
        for (int i = 0; i < turnAroundCount; i++) {
            attendre_contact(requestContacts[0]);
            sharedSection->request(loco, loco.priority);

            attendre_contact(requestContacts[1]);
            sharedSection->access(loco, loco.priority);

            for (auto [switchNumber, switchDirection] : switchSetups)
            {
                diriger_aiguillage(switchNumber, switchDirection, 0);
            }

            attendre_contact(requestContacts[3]);
            sharedSection->leave(loco);

            attendre_contact(stationContactId);
            sharedStation->enterStation(loco);

            loco.afficherMessage(QString::fromStdString("J'ai atteint le contact " + std::to_string(stationContactId)));
        }

        std::reverse(requestContacts.begin(), requestContacts.end());
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
