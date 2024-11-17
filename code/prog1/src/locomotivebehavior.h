//    ___  _________    ___  ___  ___ ____ //
//   / _ \/ ___/ __ \  |_  |/ _ \|_  / / / //
//  / ___/ /__/ /_/ / / __// // / __/_  _/ //
// /_/   \___/\____/ /____/\___/____//_/   //
//

#ifndef LOCOMOTIVEBEHAVIOR_H
#define LOCOMOTIVEBEHAVIOR_H

#include <utility>

#include "locomotive.h"
#include "launchable.h"
#include "sharedsectioninterface.h"
#include "pcosynchro/pcosemaphore.h"

struct SwitchSetup {
    int switchNumber;
    int switchDirection;
};

/**
 * @brief La classe LocomotiveBehavior représente le comportement d'une locomotive
 */
class LocomotiveBehavior : public Launchable {
public:
    /*!
     * \brief locomotiveBehavior Constructeur de la classe
     * \param loco la locomotive dont on représente le comportement
     */
    LocomotiveBehavior(Locomotive &loco, std::shared_ptr<SharedSectionInterface> sharedSection, int stationContactId,
                       int beforeSharedSectionContactId, int afterSharedSectionContactId,
                       int turnAroundCount, std::vector<SwitchSetup> switchSetups/*, autres paramètres éventuels */)
            : loco(loco),
              sharedSection(std::move(sharedSection)),
              beforeSharedSectionContactId(beforeSharedSectionContactId),
              afterSharedSectionContactId(afterSharedSectionContactId),
              stationContactId(stationContactId),
              switchSetups(switchSetups),
              turnAroundCount(turnAroundCount) {
        // Eventuel code supplémentaire du constructeur
    }

protected:
    /*!
     * \brief run Fonction lancée par le thread, représente le comportement de la locomotive
     */
    void run() override;

    /*!
     * \brief printStartMessage Message affiché lors du démarrage du thread
     */
    void printStartMessage() override;

    /*!
     * \brief printCompletionMessage Message affiché lorsque le thread a terminé
     */
    void printCompletionMessage() override;

    /**
     * @brief loco La locomotive dont on représente le comportement
     */
    Locomotive &loco;

    /**
     * @brief sharedSection Pointeur sur la section partagée
     */
    std::shared_ptr<SharedSectionInterface> sharedSection;

    /*
     * Vous êtes libres d'ajouter des méthodes ou attributs
     *
     * Par exemple la priorité ou le parcours
     */

private:
    /**
     * The id of the contact right before the shared section for this locomotive
     */
    int beforeSharedSectionContactId;

    int afterSharedSectionContactId;

    /**
     * The id of the contact that represents the locomotive station
     */
    int stationContactId;

    /**
     * The number of times the station has been reached by the locomotive
     */
    int stationReachedCount;

    std::vector<SwitchSetup> switchSetups;

    /**
     * The number of times the train has to pass over his station before turning around.
     */
    const int turnAroundCount;

    /**
     * This semaphore is used to wait on the other train when waiting at the station.
     */
    static PcoSemaphore releaseAtStation;

    /**
     * Tells us whether or not we should wait at the station (whether we are the first train waiting or not)
     */
    static bool waitAtStation;

    /**
     * This mutex protects `waitAtStation`.
     */
    static PcoSemaphore waitAtStationMutex;
};

#endif // LOCOMOTIVEBEHAVIOR_H
