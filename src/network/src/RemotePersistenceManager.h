#ifndef STI_NETWORK_REMOTEPERSISTENCECOLLECTOR_H
#define STI_NETWORK_REMOTEPERSISTENCECOLLECTOR_H

#include "deviceNet.h"
#include "PersistenceManager.h"
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

    bool saveShot(const STI::Engine::ShotID& sid, const std::shared_ptr<STI::Engine::EventEngine>& eventEngine);
    bool transferResults(const std::shared_ptr<STI::Engine::ResultsCollector>& resultsCollector);
    bool getResultTicket(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::ResultTicket>& ticket);
	void setFileHolderFactory(const std::shared_ptr<STI::Utils::FileHolderFactory>& factory);

    void setShotRepository(const std::shared_ptr<STI::Engine::ShotRepository>& repo) {}
    bool getShotRepository(std::shared_ptr<STI::Engine::ShotRepository>& repo) { return false; }

    bool ping() const;

private:

	mutable std::mutex persistenceMutex;

};


} //Network
} //STI


#endif





