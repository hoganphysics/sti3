#ifndef STI_NETWORK_REMOTEPERSISTENCECOLLECTOR_H
#define STI_NETWORK_REMOTEPERSISTENCECOLLECTOR_H

#include "deviceNet.h"
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

    bool getParseResult(const STI::Engine::ParseID& pid, std::shared_ptr<STI::Engine::ParseResult>& parseResult);
    bool getShotResult(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::ShotResult>& shotResult);

    bool saveShot(const STI::Engine::ShotID& sid, const std::shared_ptr<STI::Engine::FullShotResult>& fullShotResult, bool isOwner);

    STI::Engine::ShotResultRecord transferResults(const std::shared_ptr<STI::Engine::ResultsCollector>& resultsCollector);

    bool getMeasurements(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::MeasurementVector>& measurements);

	void setFileHolderFactory(const std::shared_ptr<STI::Utils::FileHolderFactory>& factory);
    std::shared_ptr<STI::Utils::FileHolder> makeFileHolder(const std::string& filename);

    bool ping() const;

private:

	mutable std::mutex persistenceMutex;

};


} //Network
} //STI


#endif





