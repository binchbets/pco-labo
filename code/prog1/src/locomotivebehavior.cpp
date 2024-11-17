//    ___  _________    ___  ___  ___ ____ //
//   / _ \/ ___/ __ \  |_  |/ _ \|_  / / / //
//  / ___/ /__/ /_/ / / __// // / __/_  _/ //
// /_/   \___/\____/ /____/\___/____//_/   //
//

#include "locomotivebehavior.h"
#include "ctrain_handler.h"

void LocomotiveBehavior::run()
{
    //Initialisation de la locomotive
    loco.allumerPhares();
    loco.demarrer();
    loco.afficherMessage("Ready!");

    while (true)
    {
        attendre_contact(beforeSharedSectionContactId);
        sharedSection->access(loco);

        for (auto switchSetup : switchSetups)
        {
            diriger_aiguillage(switchSetup.switchNumber, switchSetup.switchDirection, 0);
        }

        attendre_contact(afterSharedSectionContactId);
        sharedSection->leave(loco);

        // Get to the station
        attendre_contact(stationContactId);

        stationReachedCount++;
        if (stationReachedCount % turnAroundCount == 0)
        {
            loco.inverserSens();
        }

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
