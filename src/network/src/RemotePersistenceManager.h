#ifndef STI_NETWORK_REMOTEPERSISTENCEMANAGER_H
#define STI_NETWORK_REMOTEPERSISTENCEMANAGER_H

#include "generated/deviceNet.h"
#include <sti/device/PersistenceManager.h>
#include "TReferenceHolder.h"

#include <memory>
#include <mutex>

namespace STI
{
namespace Network
{

class RemotePersistenceManager : public STI::Device::PersistenceManager,
							     public STI::TNetwork::TReferenceHolder<STI::TNetwork::TPersistenceManager>	//mixin
{
public:

	RemotePersistenceManager(::STI::TNetwork::TPersistenceManager_ptr manager);
    ~RemotePersistenceManager();

    bool findShot(const STI::Engine::ShotID& sid);

    bool getParseResult(const STI::Engine::ParseID& pid, std::shared_ptr<STI::Engine::ParseResult>& parseResult);
    bool getShotResult(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::ShotResult>& shotResult);
    bool getSequenceResult(const STI::Engine::SequenceID& id, std::shared_ptr<STI::Engine::SequenceResult>& sequenceResult);

    bool saveShot(const STI::Engine::ShotID& sid, const std::shared_ptr<STI::Engine::FullShotResult>& fullShotResult, bool isOwner);

    STI::Engine::ShotResultRecord transferResults(const std::shared_ptr<STI::Engine::ResultsCollector>& resultsCollector);

    void setResultsCollectorFactory(const std::shared_ptr<STI::Engine::ResultsCollectorFactory>& factory) {}

    bool getMeasurements(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::MeasurementMap>& measurements);

	void setFileHolderFactory(const std::shared_ptr<STI::Utils::FileHolderFactory>& factory);
    void setVirtualFileServerFactory(const std::shared_ptr<STI::Utils::VirtualFileServerFactory>& factory) {}

    std::shared_ptr<STI::Utils::FileHolder> makeFileHolder(const std::string& path, const std::string& filename);
    std::shared_ptr<STI::Utils::FileHolder> makeVirtualFileHolder(const STI::Utils::FileID& fileID);
    std::shared_ptr<STI::Utils::VirtualFileServer> makeVirtualFileServer();

    void setFileServer(const std::shared_ptr<STI::Utils::FileServer>& server) {}
    bool getFileServer(std::shared_ptr<STI::Utils::FileServer>& server) { return false; }

    void addSequence(const std::shared_ptr<STI::Engine::SequenceResult>& sequenceResult);
    bool updateSequence(const STI::Engine::SequenceEntryID& id, const STI::Engine::ShotID& shotID, const STI::Engine::EngineJobStatus& shotStatus, bool isOwner);
    bool saveSequence(const std::shared_ptr<STI::Engine::SequenceResult>& sequenceResult, bool isOwner);

    bool ping() const;

private:

    std::shared_ptr<STI::Utils::FileHolderFactory> fileFactory;

	mutable std::mutex persistenceMutex;
};


} //Network
} //STI


#endif

