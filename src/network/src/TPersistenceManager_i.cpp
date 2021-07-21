
#include "TPersistenceManager_i.h"
#include "ORBManager.h"
#include "Convert_EventEngine.h"
#include "Convert_ResultsCollector.h"
#include "Convert_ResultTicket.h"
#include "PersistenceManager.h"


using STI::TNetwork::TPersistenceManager_i;
using STI::Network::convert;
using ::STI::TNetwork::TShotID;
using STI::Engine::ShotID;
using STI::Engine::EventEngine;
using STI::Engine::ResultsCollector;



TPersistenceManager_i::TPersistenceManager_i(const std::shared_ptr<STI::Device::Device>& device)
{
    if (device != 0) {
        device->getPersistenceManager(persistenceManager);        
    }
}

TPersistenceManager_i::~TPersistenceManager_i()
{
    STI::Network::ORBManager::ORBManager::deactivateServant(this);
}

::CORBA::Boolean TPersistenceManager_i::saveShot(const ::STI::TNetwork::TShotID& sid, ::STI::TNetwork::TEventEngine_ptr eventEngine)
{
	std::shared_ptr<EventEngine> engine;
	bool success = convert<::STI::TNetwork::TEventEngine_var, std::shared_ptr<EventEngine>>(eventEngine, engine);
    
	if (persistenceManager != 0 && success) {

		success &= persistenceManager->saveShot(convert<TShotID, ShotID>(sid), engine);
	}
	else {
		success = false;
	}
	return success;
}

::CORBA::Boolean TPersistenceManager_i::transferMeasurements(::STI::TNetwork::TResultsCollector_ptr resultsCollector)
{
	std::shared_ptr<ResultsCollector> remoteCollector;
	bool success = convert<::STI::TNetwork::TResultsCollector_var, std::shared_ptr<ResultsCollector>>(resultsCollector, remoteCollector);
    
	if (persistenceManager != 0 && success) {

		success &= persistenceManager->transferResults(remoteCollector);
	}
	else {
		success = false;
	}

	return success;
}

::CORBA::Boolean TPersistenceManager_i::getResultTicket(const ::STI::TNetwork::TShotID& sid, ::STI::TNetwork::TResultTicket_out ticket)
{
	bool success = false;

    if (persistenceManager != 0) {

		STI::TNetwork::TResultTicket_var tResultsTicket_var(new STI::TNetwork::TResultTicket);
		std::shared_ptr<STI::Engine::ResultTicket> resultsTicket;

		success = persistenceManager->getResultTicket(convert<TShotID, ShotID>(sid), resultsTicket);

        if (success) {

            success = convert<std::shared_ptr<STI::Engine::ResultTicket>, STI::TNetwork::TResultTicket>(resultsTicket, tResultsTicket_var);

    		ticket = new STI::TNetwork::TResultTicket();
	    	(*ticket) = tResultsTicket_var;
        }
	}

	return success;
}

::CORBA::Boolean TPersistenceManager_i::ping()
{
    return true;
}

