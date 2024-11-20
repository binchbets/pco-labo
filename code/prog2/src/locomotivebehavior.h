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
#include "sharedstation.h"
#include "pcosynchro/pcosemaphore.h"

struct SwitchSetup
{
    int switchNumber;
    int switchDirection;
};

/**
 * @brief La classe LocomotiveBehavior représente le comportement d'une locomotive
 */
class LocomotiveBehavior : public Launchable
{
public:
    /*!
     * \brief locomotiveBehavior Constructeur de la classe
     * \param loco la locomotive dont on représente le comportement
     */
    LocomotiveBehavior(Locomotive& loco, std::shared_ptr<SharedSectionInterface> sharedSection,
                       std::shared_ptr<SharedStation> sharedStation, int stationContactId,
                       int beforeSharedSectionContactId, int afterSharedSectionContactId, int turnAroundCount,
                       std::vector<SwitchSetup> switchSetups/*, autres paramètres éventuels */)
        : loco(loco),
          sharedSection(std::move(sharedSection)),
          sharedStation(std::move(sharedStation)),
          beforeSharedSectionContactId(beforeSharedSectionContactId),
          afterSharedSectionContactId(afterSharedSectionContactId),
          turnAroundCount(turnAroundCount),
          stationContactId(stationContactId),
          switchSetups(std::move(switchSetups))
    {
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
    Locomotive& loco;

    /**
     * @brief sharedSection Pointeur sur la section partagée
     */
    std::shared_ptr<SharedSectionInterface> sharedSection;
    std::shared_ptr<SharedStation> sharedStation;

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

    int turnAroundCount;

    /**
     * The id of the contact that represents the locomotive station
     */
    int stationContactId;

    std::vector<SwitchSetup> switchSetups;

    /**
     * This semaphore is used to wait on the other train when waiting at the station.
     */
    static PcoSemaphore releaseAtStation;

    /**
     * This mutex protects `waitAtStation`.
     */
    static PcoSemaphore waitAtStationMutex;
};

#endif // LOCOMOTIVEBEHAVIOR_H
