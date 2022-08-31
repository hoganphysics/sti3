

#ifndef STI_DEVICE_LOCALPERSISTENCEMANAGER_H
#define STI_DEVICE_LOCALPERSISTENCEMANAGER_H

#include <sti/device/PersistenceManager.h>
#include <sti/utils/SynchronizedMap.h>
#include <sti/device/DeviceCollection.h>
#include "ShotRepository.h"
#include "FullShotResult.h"

#include "utils/OrderedBufferMap.h"

#include <memory>


namespace STI
{
namespace Device
{

class DeviceID;


class LocalPersistenceManager : public PersistenceManager
{
public:

    LocalPersistenceManager(const DeviceID& deviceID, const std::string& basePath, 
        const std::shared_ptr<STI::Utils::FileHolderFactory>& fileHolderFactory, 
        const std::shared_ptr<STI::Device::DeviceCollection>& collection);
    ~LocalPersistenceManager();

    bool getParseResult(const STI::Engine::ParseID& pid, std::shared_ptr<STI::Engine::ParseResult>& parseResult);

    bool getShotResult(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::ShotResult>& result);
    bool saveShot(const STI::Engine::ShotID& sid, const std::shared_ptr<STI::Engine::FullShotResult>& fullShotResult, bool isOwner);

    STI::Engine::ShotResultRecord transferResults(const std::shared_ptr<STI::Engine::ResultsCollector>& resultsCollector);

    bool getMeasurements(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::MeasurementVector>& measurements);

	void setFileHolderFactory(const std::shared_ptr<STI::Utils::FileHolderFactory>& factory);

    std::shared_ptr<STI::Utils::FileHolder> makeFileHolder(const std::string& filename);

    //bool getResultTicket(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::ResultTicket>& ticket);
    //std::shared_ptr<STI::Engine::ResultsCollector> createResultsCollector(const STI::Engine::ShotID& sid, const std::shared_ptr<STI::Engine::EventEngine>& eventEngine);

    // void saveShot(const std::shared_ptr<STI::Engine::ResultsCollector>& collector);

    // void setResultsCollectorFactory(const std::shared_ptr<STI::Engine::ResultsCollectorFactory>& factory) {}

    // void saveShot(const std::shared_ptr<STI::Engine::ResultsCollector>& collector) {}
    // void copyShot(const std::shared_ptr<STI::Engine::ResultsCollector>& collector) {}

    void addPersistenceDelegate(const DeviceID& id, const std::shared_ptr<PersistenceManager>& manager, unsigned priority);
    void removePersistenceDelegate(const DeviceID& id);

    void setShotRepository(const std::shared_ptr<STI::Engine::ShotRepository>& repo);
    bool getShotRepository(std::shared_ptr<STI::Engine::ShotRepository>& repo);

    // std::string getBaseLocalPath();

    static std::string makeBasePath(const std::string& rootPath, const DeviceID& deviceID);

private:

    bool getShotLocal(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::ShotResult>& result);
    bool saveShotLocal(const STI::Engine::ShotID& sid, const std::shared_ptr<STI::Engine::FullShotResult>& shotResult, bool isOwner);

    STI::Engine::ShotResultRecord transferResults(const std::shared_ptr<STI::Engine::ResultsCollector>& resultsCollector, 
                         const std::shared_ptr<STI::Engine::ShotResult>& shotResult, bool transferDependents);

    bool addToBuffer(const std::shared_ptr<STI::Engine::FullShotResult>& shotResult);

        //std::shared_ptr<STI::Engine::ResultsDocumenter> resultsDocumenter;
        
    std::shared_ptr<STI::Engine::ShotRepository> defaultRepository;
    std::shared_ptr<STI::Engine::ShotRepository> shotRepository;
    std::shared_ptr<STI::Engine::ShotRepository> transientRepository;

//        std::shared_ptr<STI::Engine::ShotRepository> bufferRepository;  //for temporary storage
    STI::Utils::OrderedBufferMap<STI::Engine::ShotID, std::shared_ptr<STI::Engine::FullShotResult>> resultBuffer;     //for temporary storage, before serializing to default repo or saving

    STI::Utils::SynchronizedMap<unsigned, DeviceID> delegatePriorities;
    STI::Utils::SynchronizedMap<DeviceID, std::shared_ptr<PersistenceManager>> delegates;

    std::shared_ptr<STI::Utils::FileHolderFactory> fileHolderFactory;
    std::shared_ptr<STI::Device::DeviceCollection> deviceCollection;
    DeviceID localDeviceID;
};


} //Device
} //STI

#endif
