
#include "RemotePersistenceManager.h"

#include "Convert_ResultsCollector.h"
#include "Convert_EventEngine.h"
#include "Convert_ResultTicket.h"
#include "orbTypes.h"
#include "NetworkResultsCollector.h"
#include "RawEvent.h"

using STI::Network::NetworkResultsCollector;
using STI::Network::RemotePersistenceManager;
using STI::TNetwork::TReferenceHolder;
using STI::Network::convert;
using ::STI::TNetwork::TPersistenceManager;
using STI::Engine::ResultTicket;
using STI::TNetwork::TResultTicket;
using STI::TNetwork::TEventEngine_ptr;
using STI::Engine::ResultsCollector;
using STI::Engine::EventEngine;


RemotePersistenceManager::RemotePersistenceManager(::STI::TNetwork::TPersistenceManager_ptr manager)
: TReferenceHolder<TPersistenceManager>(manager, persistenceMutex)
{
}

RemotePersistenceManager::~RemotePersistenceManager()
{
}

bool RemotePersistenceManager::saveShot(const STI::Engine::ShotID& sid, const std::shared_ptr<STI::Engine::EventEngine>& eventEngine)
{
	std::unique_lock<std::mutex> persistenceLock(persistenceMutex);
	
    if (isDisabled()) return false;
    
    bool success = false;
   	STI::TNetwork::TEventEngine_var tEventEngine;

	try {

        success &= convert<std::shared_ptr<EventEngine>, STI::TNetwork::TEventEngine_var>(eventEngine, tEventEngine);

        if (success) {
    		success = getTRef()->saveShot(convert<STI::Engine::ShotID, STI::TNetwork::TShotID>(sid), tEventEngine);	//remote call
        }

	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}

    return success && (eventEngine != 0);
}

bool RemotePersistenceManager::transferResults(const std::shared_ptr<STI::Engine::ResultsCollector>& resultsCollector)
{
	std::unique_lock<std::mutex> persistenceLock(persistenceMutex);
	
    if (isDisabled()) return false;
    
    bool success = false;
   	STI::TNetwork::TResultsCollector_var tResultsCollector;

	try {

        if (NetworkResultsCollector::getTResultsCollector(resultsCollector, tResultsCollector)) {
		    success = getTRef()->transferMeasurements(tResultsCollector);	//remote call            
        }
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}

    return success;
}



bool RemotePersistenceManager::getResultTicket(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::ResultTicket>& ticket)
{
	std::unique_lock<std::mutex> persistenceLock(persistenceMutex);
	
    if (isDisabled()) return false;
    
    bool success = false;
   	STI::TNetwork::TResultTicket_var tResultTicket(new TResultTicket);

	try {

		success = getTRef()->getResultTicket(convert<STI::Engine::ShotID, STI::TNetwork::TShotID>(sid), tResultTicket);	//remote call

        if (success) {
            success &= convert<STI::TNetwork::TResultTicket, std::shared_ptr<ResultTicket>>(tResultTicket.in(), ticket);
        }
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}

    return success && (ticket != 0);
}

void RemotePersistenceManager::setFileHolderFactory(const std::shared_ptr<STI::Utils::FileHolderFactory>& factory)
{
}

bool RemotePersistenceManager::ping() const
{
	std::unique_lock<std::mutex> persistenceLock(persistenceMutex);

	if (isDisabled()) return false;

	bool success = false;

	try {
		success = getTRef()->ping();	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	return success;

}
