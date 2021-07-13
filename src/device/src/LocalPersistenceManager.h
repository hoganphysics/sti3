

#ifndef STI_DEVICE_LOCALPERSISTENCEMANAGER_H
#define STI_DEVICE_LOCALPERSISTENCEMANAGER_H

#include "PersistenceManager.h"
#include "SynchronizedMap.h"
#include "ResultsDocumenter.h"
#include "ShotRepository.h"

#include <memory>


namespace STI
{
namespace Device
{

class DeviceID;


class LocalPersistenceManager : public PersistenceManager
{
public:

    LocalPersistenceManager(const std::shared_ptr<STI::Utils::FileHolderFactory>& fileHolderFactory,
        const std::shared_ptr<STI::Engine::ResultsDocumenter>& resultsDocumenter,
        const std::shared_ptr<STI::Engine::ShotRepository>& shotRepository);


    bool saveShot(const STI::Engine::ShotID& sid, const std::shared_ptr<STI::Engine::EventEngine>& eventEngine);
    bool transferMeasurements(const std::shared_ptr<STI::Engine::ResultsCollector>& resultsCollector);



    bool getResultTicket(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::ResultTicket>& ticket);
    //std::shared_ptr<STI::Engine::ResultsCollector> createResultsCollector(const STI::Engine::ShotID& sid, const std::shared_ptr<STI::Engine::EventEngine>& eventEngine);

    // void saveShot(const std::shared_ptr<STI::Engine::ResultsCollector>& collector);

    // void setResultsCollectorFactory(const std::shared_ptr<STI::Engine::ResultsCollectorFactory>& factory) {}

    // void saveShot(const std::shared_ptr<STI::Engine::ResultsCollector>& collector) {}
    // void copyShot(const std::shared_ptr<STI::Engine::ResultsCollector>& collector) {}

    void addPersistenceDelegate(const DeviceID& id, const std::shared_ptr<PersistenceManager>& manager, unsigned priority);
    void removePersistenceDelegate(const DeviceID& id);

	void setFileHolderFactory(const std::shared_ptr<STI::Utils::FileHolderFactory>& factory);
    void setResultsDocumenter(const std::shared_ptr<STI::Engine::ResultsDocumenter>& documenter);

    void setShotRepository(const std::shared_ptr<STI::Engine::ShotRepository>& repo);
    bool getShotRepository(std::shared_ptr<STI::Engine::ShotRepository>& repo);

private:

    bool saveShotLocal(const STI::Engine::ShotID& sid, const std::shared_ptr<STI::Engine::EventEngine>& eventEngine);

    std::shared_ptr<STI::Engine::ResultsDocumenter> resultsDocumenter;
    std::shared_ptr<STI::Engine::ShotRepository> shotRepository;

    STI::Utils::SynchronizedMap<unsigned, DeviceID> delegatePriorities;
    STI::Utils::SynchronizedMap<DeviceID, std::shared_ptr<PersistenceManager>> delegates;

	std::shared_ptr<STI::Utils::FileHolderFactory> fileHolderFactory;
};


} //Device
} //STI

#endif
