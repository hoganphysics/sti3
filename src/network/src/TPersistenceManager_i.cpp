
#include "TPersistenceManager_i.h"
#include "ORBManager.h"
#include "Convert_EventEngine.h"
#include "Convert_ResultsCollector.h"
#include "Convert_ShotResult.h"
#include <sti/device/PersistenceManager.h>
#include <sti/engine/ShotResultRecord.h>


using STI::TNetwork::TPersistenceManager_i;
using STI::Network::convert;
using ::STI::TNetwork::TShotID;
using STI::Engine::ShotID;
using STI::Engine::ShotResult;
using STI::Engine::ResultsCollector;
using ::STI::TNetwork::TShotResultRecord;
using STI::Engine::ShotResultRecord;


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

::CORBA::Boolean TPersistenceManager_i::getShot(const ::STI::TNetwork::TShotID& sid, ::STI::TNetwork::TShotResult_out tShotResult)
{
	bool success = false;

    if (persistenceManager != 0) {

		std::shared_ptr<STI::Engine::ShotResult> shotResult;
		STI::TNetwork::TShotResult_var tShotResult_var(new STI::TNetwork::TShotResult);

		success = persistenceManager->getShot(convert<TShotID, STI::Engine::ShotID>(sid), shotResult);

		success &= convert<std::shared_ptr<STI::Engine::ShotResult>, TShotResult>(
					shotResult, tShotResult_var);

		tShotResult = new STI::TNetwork::TShotResult();
		(*tShotResult) = tShotResult_var;		
	}

	return success;
}


::CORBA::Boolean TPersistenceManager_i::saveShot(const ::STI::TNetwork::TShotID& sid, 
												 const ::STI::TNetwork::TShotResult& tShotResult, ::CORBA::Boolean isOwner)
{
	std::shared_ptr<ShotResult> shotResult;
	bool success = convert<::STI::TNetwork::TShotResult, std::shared_ptr<ShotResult>>(tShotResult, shotResult);
    
	if (persistenceManager != 0 && success) {

		success &= persistenceManager->saveShot(convert<TShotID, ShotID>(sid), shotResult, isOwner);
	}
	else {
		success = false;
	}
	return success;
}


TShotResultRecord* TPersistenceManager_i::transferResults(::STI::TNetwork::TResultsCollector_ptr tResultsCollector)
{
	std::shared_ptr<ResultsCollector> remoteCollector;
	bool success = convert<::STI::TNetwork::TResultsCollector_var, std::shared_ptr<ResultsCollector>>(tResultsCollector, remoteCollector);
    
	ShotResultRecord record;
	STI::TNetwork::TShotResultRecord_var tShotResultRecord(new STI::TNetwork::TShotResultRecord);

	if (persistenceManager != 0 && success) {

		record = persistenceManager->transferResults(remoteCollector);

		convert<ShotResultRecord, TShotResultRecord>(record, tShotResultRecord);
	}
	else {
		record.recordStatus = STI::Engine::RecordStatus::Error;
	}

	return tShotResultRecord._retn();
}

::CORBA::Boolean TPersistenceManager_i::getMeasurements(const ::STI::TNetwork::TShotID& sid, ::STI::TNetwork::TMeasurementSeq_out measurements)
{
	bool success = false;

    if (persistenceManager != 0) {

		STI::TNetwork::TMeasurementSeq_var tMeasurementSeq_var(new STI::TNetwork::TMeasurementSeq);
		auto localMeasurements = std::make_shared<STI::Engine::MeasurementVector>();

		success = persistenceManager->getMeasurements(convert<STI::TNetwork::TShotID, STI::Engine::ShotID>(sid), localMeasurements);

		success &= convert<std::shared_ptr<STI::Engine::Measurement>, STI::TNetwork::TMeasurement>(*localMeasurements,
					(_CORBA_Unbounded_Sequence<STI::TNetwork::TMeasurement>&) tMeasurementSeq_var);

		// success &= convert<STI::Engine::MeasurementVector, ::STI::TNetwork::TMeasurementSeq>(deviceEvents, tDeviceEventsSeq_var);	
		
		measurements = new STI::TNetwork::TMeasurementSeq();
		(*measurements) = tMeasurementSeq_var;
	}

	return success;
}



// ::CORBA::Boolean TPersistenceManager_i::getResultTicket(const ::STI::TNetwork::TShotID& sid, ::STI::TNetwork::TResultTicket_out ticket)
// {
// 	bool success = false;

//     if (persistenceManager != 0) {

// 		STI::TNetwork::TResultTicket_var tResultsTicket_var(new STI::TNetwork::TResultTicket);
// 		std::shared_ptr<STI::Engine::ResultTicket> resultsTicket;

// 		success = persistenceManager->getResultTicket(convert<TShotID, ShotID>(sid), resultsTicket);

//         if (success) {

//             success = convert<std::shared_ptr<STI::Engine::ResultTicket>, STI::TNetwork::TResultTicket>(resultsTicket, tResultsTicket_var);

//     		ticket = new STI::TNetwork::TResultTicket();
// 	    	(*ticket) = tResultsTicket_var;
//         }
// 	}

// 	return success;
// }

::CORBA::Boolean TPersistenceManager_i::ping()
{
    return true;
}

