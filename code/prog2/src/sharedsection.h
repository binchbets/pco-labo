//    ___  _________    ___  ___  ___ ____ //
//   / _ \/ ___/ __ \  |_  |/ _ \|_  / / / //
//  / ___/ /__/ /_/ / / __// // / __/_  _/ //
// /_/   \___/\____/ /____/\___/____//_/   //
//                                         //


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
class SharedSection final : public SharedSectionInterface
{
public:

    /**
     * @brief SharedSection Constructeur de la classe qui représente la section partagée.
     * Initialisez vos éventuels attributs ici, sémaphores etc.
     */
    SharedSection() :
            isOccupied(false),
            isWaiting(false),
            mutex(1),
            lockUntilFree(0) {
        // TODO
    }

    /**
     * @brief request Méthode a appeler pour indiquer que la locomotive désire accéder à la
     * section partagée (deux contacts avant la section partagée).
     * @param loco La locomotive qui désire accéder
     * @param locoId L'identidiant de la locomotive qui fait l'appel
     * @param entryPoint Le point d'entree de la locomotive qui fait l'appel
     */
    void request(Locomotive& loco, int priority) override {
        // TODO

        // Exemple de message dans la console globale
        afficher_message(qPrintable(QString("The engine no. %1 requested the shared section.").arg(loco.numero())));
    }

    /**
     * @brief access Méthode à appeler pour accéder à la section partagée, doit arrêter la
     * locomotive et mettre son thread en attente si la section est occupée par une autre locomotive.
     * Si la locomotive et son thread ont été mis en attente,
     * le thread doit être reveillé lorsque la section partagée est à nouveau libre et
     * la locomotive redémarée. (méthode à appeler un contact avant la section partagée).
     * @param loco La locomotive qui essaie accéder à la section partagée
     */
    void access(Locomotive &loco, int priority) override {
        // TODO
        mutex.acquire();
        if (isOccupied) {
            loco.arreter();

            isWaiting = true;
            mutex.release();

            afficher_message(qPrintable(QString("The train no. %1 is waiting at the shared section.").arg(loco.numero())));

            lockUntilFree.acquire();
            loco.demarrer();

            afficher_message(qPrintable(QString("The train no. %1 was release and can proceed to the shared section").arg(loco.numero())));
        } else {
            isOccupied = true;
            mutex.release();

            afficher_message(qPrintable(QString("The train no. %1 accesses the shared section.").arg(loco.numero())));
        }
    }

    /**
     * @brief leave Méthode à appeler pour indiquer que la locomotive est sortie de la section
     * partagée. (reveille les threads des locomotives potentiellement en attente).
     * @param loco La locomotive qui quitte la section partagée
     */
    void leave(Locomotive &loco) override {
        mutex.acquire();
        isOccupied = false;

        // We only have to release `lockUntilFree` if there is a train waiting.
        if (isWaiting) {
            lockUntilFree.release();
        }
        isWaiting = false;
        mutex.release();

        afficher_message(qPrintable(QString("The engine no. %1 leaves the shared section.").arg(loco.numero())));
    }

    void togglePriorityMode() {
        /* TODO */
    }

private:

    /* A vous d'ajouter ce qu'il vous faut */

    /**
     * Tells us whether or not the shared section is occupied.
     */
    bool isOccupied;
    /**
     * Tells us whether or there is a train waiting at the start of the shared section.
     */
    bool isWaiting;
    PcoSemaphore mutex;

    /**
     * Locks the caller of access (LocomotiveBehavior) until the shared section is free.
     */
    PcoSemaphore lockUntilFree;

    // Méthodes privées ...
    // Attribut privés ...
};


#endif // SHAREDSECTION_H
