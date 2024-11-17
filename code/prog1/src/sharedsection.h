//    ___  _________    ___  ___  ___ ____ //
//   / _ \/ ___/ __ \  |_  |/ _ \|_  / / / //
//  / ___/ /__/ /_/ / / __// // / __/_  _/ //
// /_/   \___/\____/ /____/\___/____//_/   //
//

#ifndef SHAREDSECTION_H
#define SHAREDSECTION_H

#include <QDebug>

#include <pcosynchro/pcosemaphore.h>

#include "locomotive.h"
#include "ctrain_handler.h"
#include "sharedsectioninterface.h"

/**
 * @brief La classe SharedSection implémente l'interface SharedSectionInterface qui
 * propose les méthodes liées à la section partagée.
 */
class SharedSection final : public SharedSectionInterface {
public:

    /**
     * @brief SharedSection Constructeur de la classe qui représente la section partagée.
     * Initialisez vos éventuels attributs ici, sémaphores etc.
     */
    SharedSection(std::vector<std::tuple<int, int, int>> &switchingOperations) :
            switchingOperations(switchingOperations),
            isOccupied(false),
            isWaiting(false),
            isOccupiedMutex(1),
            lockUntilFree(0) {
        // TODO
    }

    /**
     * @brief access Méthode à appeler pour accéder à la section partagée, doit arrêter la
     * locomotive et mettre son thread en attente si la section est occupée par une autre locomotive.
     * Si la locomotive et son thread ont été mis en attente,
     * le thread doit être reveillé lorsque la section partagée est à nouveau libre et
     * la locomotive redémarée. (méthode à appeler un contact avant la section partagée).
     * @param loco La locomotive qui essaie accéder à la section partagée
     */
    void access(Locomotive &loco) override {
        // TODO

        isOccupiedMutex.acquire();
        if (isOccupied) {
            loco.arreter();

            isWaiting = true;

            isOccupiedMutex.release();

            afficher_message(qPrintable(QString("The train no. %1 is waiting at the shared section.").arg(loco.numero())));

            lockUntilFree.acquire();

            loco.demarrer();

            afficher_message(qPrintable(QString("The train no. %1 was release and can proceed to the shared section").arg(loco.numero())));
        } else {
            isOccupied = true;
            isOccupiedMutex.release();

            afficher_message(qPrintable(QString("The train no. %1 accesses the shared section.").arg(loco.numero())));
        }
    }

    /**
     * @brief leave Méthode à appeler pour indiquer que la locomotive est sortie de la section
     * partagée. (reveille les threads des locomotives potentiellement en attente).
     * @param loco La locomotive qui quitte la section partagée
     */
    void leave(Locomotive &loco) override {
        // TODO
        isOccupiedMutex.acquire();
        isOccupied = false;

        if (isWaiting) {
            lockUntilFree.release();
        }
        isWaiting = false;

        isOccupiedMutex.release();


        afficher_message(qPrintable(QString("The engine no. %1 leaves the shared section.").arg(loco.numero())));
    }

private:

    /* A vous d'ajouter ce qu'il vous faut */

    /**
     * Contains a pair of (contact point, switch) that are used to toggle the switch when a train passes over a contact
     * point.
     */
    std::vector<std::tuple<int, int, int>> switchingOperations;

    /**
     * Tells us whether or not the shared section is occupied
     */
    bool isOccupied;
    bool isWaiting;
    PcoSemaphore isOccupiedMutex;


    /**
     * Locks the caller of access (LocomotiveBehavior) until the shared section is free
     */
    PcoSemaphore lockUntilFree;

    // Méthodes privées ...
    // Attribut privés ...
};


#endif // SHAREDSECTION_H
