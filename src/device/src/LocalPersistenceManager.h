#ifndef STI_DEVICE_LOCALPERSISTENCEMANAGER_H
#define STI_DEVICE_LOCALPERSISTENCEMANAGER_H

#include <sti/device/DeviceCollection.h>
#include <sti/device/DeviceMessageListener.h>
#include <sti/device/DeviceMessage.h>
#include <sti/device/PersistenceManager.h>
#include <sti/engine/EventEngineScheduler.h>
#include <sti/engine/FullShotResult.h>
#include <sti/utils/SynchronizedMap.h>

#include <sti/engine/ShotRepository.h>
#include "utils/OrderedBufferMap.h"
#include <sti/utils/Configuration.h>

#include <memory>


namespace STI
{
namespace Device
{

class DeviceID;
class PersistenceTarget;
class PersistenceTargetHolder;


class LocalPersistenceManager : public PersistenceManager,
                                public STI::Device::DeviceMessageListener<STI::Device::EngineSchedulerMessage>
{
public:

    LocalPersistenceManager(const DeviceID& deviceID, const STI::Utils::Configuration& config, const std::string& basePath, 
        const std::shared_ptr<STI::Utils::FileHolderFactory>& fileHolderFactory, 
        const std::shared_ptr<STI::Device::DeviceCollection>& collection);
    ~LocalPersistenceManager();

    bool findShot(const STI::Engine::ShotID& sid);

    bool getParseResult(const STI::Engine::ParseID& pid, std::shared_ptr<STI::Engine::ParseResult>& parseResult);
    bool getShotResult(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::ShotResult>& result);
    bool getSequenceResult(const STI::Engine::SequenceID& id, std::shared_ptr<STI::Engine::SequenceResult>& sequenceResult);

    bool saveShot(const STI::Engine::ShotID& sid, const std::shared_ptr<STI::Engine::FullShotResult>& fullShotResult, bool isOwner);

    STI::Engine::ShotResultRecord transferResults(const std::shared_ptr<STI::Engine::ResultsCollector>& resultsCollector);

    bool getMeasurements(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::MeasurementMap>& measurements);

	void setFileHolderFactory(const std::shared_ptr<STI::Utils::FileHolderFactory>& factory);

    void setResultsCollectorFactory(const std::shared_ptr<STI::Engine::ResultsCollectorFactory>& factory);

    std::shared_ptr<STI::Utils::FileHolder> makeFileHolder(const std::string& filename);

    void addSequence(const std::shared_ptr<STI::Engine::SequenceResult>& sequenceResult);
    bool updateSequence(const STI::Engine::SequenceEntryID& id, const STI::Engine::ShotID& shotID, const STI::Engine::EngineJobStatus& shotStatus, bool isOwner);
    bool saveSequence(const std::shared_ptr<STI::Engine::SequenceResult>& sequenceResult, bool isOwner);

    void addPersistenceDelegate(const DeviceID& id, const std::shared_ptr<PersistenceManager>& manager, unsigned priority);
    void removePersistenceDelegate(const DeviceID& id);

    void setShotRepository(const std::shared_ptr<STI::Engine::ShotRepository>& repo);
    bool getShotRepository(std::shared_ptr<STI::Engine::ShotRepository>& repo);

    void attachEngineScheduler(const std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler);
    void addPersistenceTarget(const std::shared_ptr<PersistenceTarget>& target);
    void loadPersistenceTargets();

    std::string getBasePath() const;

    static std::string makeBasePath(const std::string& rootPath, const std::string& deviceID, bool autocreate = true);

    //log files
    bool getLogBasePath(const STI::Utils::TimeStamp& timestamp, std::string& logBasePath);  //for date in timestamp
    bool getLogBasePath(const STI::Utils::TimeStamp& timestamp, const DeviceID& deviceID, std::string& logBasePath);  //for date in timestamp
    bool makeLogPath(const STI::Utils::TimeStamp& timestamp, std::string& logPath);
    bool makeLogPath(const STI::Utils::TimeStamp& timestamp, const DeviceID& deviceID, std::string& logPath);

private:

    void handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& mess);

    bool findShotLocal(const STI::Engine::ShotID& sid);
    bool getShotLocal(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::ShotResult>& result);
    bool saveShotLocal(const STI::Engine::ShotID& sid, const std::shared_ptr<STI::Engine::FullShotResult>& shotResult, bool isOwner);

    bool getParseResultLocal(const STI::Engine::ParseID& pid, std::shared_ptr<STI::Engine::ParseResult>& parseResult);
    bool findBufferedParseResult(const STI::Engine::ParseID& pid, STI::Engine::ShotID& sid);

    bool getSequenceLocal(const STI::Engine::SequenceID& seqid, std::shared_ptr<STI::Engine::SequenceResult>& sequenceResult);
    bool updateSequenceLocal(const STI::Engine::SequenceEntryID& id, const STI::Engine::ShotID& shotID, const STI::Engine::EngineJobStatus& shotStatus, bool isOwner);
    bool saveSequenceLocal(const std::shared_ptr<STI::Engine::SequenceResult>& sequenceResult, bool isOwner);

    STI::Engine::ShotResultRecord transferResults(const std::shared_ptr<STI::Engine::ResultsCollector>& resultsCollector, 
                         const std::shared_ptr<STI::Engine::ShotResult>& shotResult, bool transferDependents);

    bool addToBuffer(const std::shared_ptr<STI::Engine::FullShotResult>& shotResult);
    bool addToBuffer(const std::shared_ptr<STI::Engine::SequenceResult>& sequenceResult);

    void transferParseResult(std::shared_ptr<STI::Engine::ParseResult> parseResult, const std::string& timingPath);

    bool getResultsPaths(const STI::Engine::ShotID& sid, STI::Engine::ResultsPaths& resultsPaths);


    std::shared_ptr<STI::Engine::ShotRepository> defaultRepository;
    std::shared_ptr<STI::Engine::ShotRepository> shotRepository;
    std::shared_ptr<STI::Engine::ShotRepository> transientRepository;

    STI::Utils::OrderedBufferMap<STI::Engine::ShotID, std::shared_ptr<STI::Engine::FullShotResult>> resultBuffer;     //for temporary storage, before serializing to default repo or saving
    STI::Utils::OrderedBufferMap<STI::Engine::SequenceID, std::shared_ptr<STI::Engine::SequenceResult>> sequenceBuffer;

    STI::Utils::SynchronizedMap<unsigned, DeviceID> delegatePriorities;
    STI::Utils::SynchronizedMap<DeviceID, std::shared_ptr<PersistenceManager>> delegates;

    std::weak_ptr<STI::Engine::EventEngineScheduler> eventEngineScheduler;

    std::shared_ptr<STI::Engine::ResultsCollectorFactory> resultsCollectorFactory;
    std::shared_ptr<STI::Utils::FileHolderFactory> fileHolderFactory;
    std::shared_ptr<STI::Device::DeviceCollection> deviceCollection;

    std::vector<std::shared_ptr<PersistenceTargetHolder>> persistenceTargetHolders;

    DeviceID localDeviceID;
    std::string basePath;
};


} //Device
} //STI

#endif
