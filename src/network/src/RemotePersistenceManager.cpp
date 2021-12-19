
#include "RemotePersistenceManager.h"

#include "Convert_ResultsCollector.h"
#include "Convert_EventEngine.h"
#include "Convert_ShotResult.h"
#include "orbTypes.h"
#include "NetworkResultsCollector.h"
#include "RawEvent.h"

using STI::Network::NetworkResultsCollector;
using STI::Network::RemotePersistenceManager;
using STI::TNetwork::TReferenceHolder;
using STI::Network::convert;
using ::STI::TNetwork::TPersistenceManager;
using STI::TNetwork::TEventEngine_ptr;
using STI::Engine::ResultsCollector;
using STI::Engine::EventEngine;
using STI::TNetwork::TShotID;
using STI::Engine::ShotID;
using STI::Engine::ShotResultRecord;
using STI::TNetwork::TShotResultRecord;
using STI::Engine::ShotResult;
using STI::TNetwork::TShotResult;


RemotePersistenceManager::RemotePersistenceManager(::STI::TNetwork::TPersistenceManager_ptr manager)
: TReferenceHolder<TPersistenceManager>(manager, persistenceMutex)
{
}

RemotePersistenceManager::~RemotePersistenceManager()
{
}

bool RemotePersistenceManager::findShot(const STI::Engine::ShotID& sid)
{
	std::unique_lock<std::mutex> persistenceLock(persistenceMutex);

	if (isDisabled()) return false;

    bool success = false;

	try {
		success = getTRef()->findShot(convert<STI::Engine::ShotID, STI::TNetwork::TShotID>(sid));	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}

    return success;
}

bool RemotePersistenceManager::getShot(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::ShotResult>& result)
{
	std::unique_lock<std::mutex> persistenceLock(persistenceMutex);

	if (isDisabled()) return false;

	STI::TNetwork::TShotResult_var tShotResult(new STI::TNetwork::TShotResult);

    bool success = false;

	try {

		success = getTRef()->getShot(
					convert<STI::Engine::ShotID, STI::TNetwork::TShotID>(sid), 
					tShotResult);	//remote call
		
		if (success) {
			success = convert<TShotResult, std::shared_ptr<ShotResult>>(tShotResult, result);
		}
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}

    return success && (result != 0);
}


bool RemotePersistenceManager::saveShot(const STI::Engine::ShotID& sid, const std::shared_ptr<ShotResult>& shotResult, bool isOwner)
{
	std::unique_lock<std::mutex> persistenceLock(persistenceMutex);
	
    if (isDisabled()) return false;
    
    bool success = false;
   	STI::TNetwork::TShotResult tShotResult;

	try {

        success &= convert<std::shared_ptr<ShotResult>, TShotResult>(shotResult, tShotResult);

        if (success) {
    		success = getTRef()->saveShot(
						convert<STI::Engine::ShotID, STI::TNetwork::TShotID>(sid), 
						tShotResult,
						static_cast<::CORBA::Boolean>(isOwner));	//remote call
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


STI::Engine::ShotResultRecord RemotePersistenceManager::transferResults(const std::shared_ptr<STI::Engine::ResultsCollector>& resultsCollector)
{
	std::unique_lock<std::mutex> persistenceLock(persistenceMutex);

	ShotResultRecord record;
	
    if (isDisabled()) return record;
    
    bool success = false;
   	STI::TNetwork::TResultsCollector_var tResultsCollector;

	try {

        if (NetworkResultsCollector::getTResultsCollector(resultsCollector, tResultsCollector)) {
		    auto tRecord = getTRef()->transferResults(tResultsCollector);	//remote call

			if (tRecord != 0) {
				record = convert<TShotResultRecord, ShotResultRecord>(*tRecord);
			}
        }
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}

    return record;
}

bool RemotePersistenceManager::getMeasurements(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::MeasurementVector>& measurements)
{
	std::unique_lock<std::mutex> persistenceLock(persistenceMutex);

	if (isDisabled()) return false;

	STI::TNetwork::TMeasurementSeq_var tMeasurements(new STI::TNetwork::TMeasurementSeq);
	measurements = std::make_shared<STI::Engine::MeasurementVector>();

	bool success = false;

	try {
		success = getTRef()->getMeasurements(convert<ShotID, TShotID>(sid), tMeasurements);	//remote call

		//success &= convert<::STI::TNetwork::TMeasurementSeq, STI::Engine::MeasurementVector>(tMeasurements, *measurements); (_CORBA_Unbounded_Sequence<::STI::TNetwork::TMeasurement>) 
		success &= convert<::STI::TNetwork::TMeasurement, std::shared_ptr<STI::Engine::Measurement>>(tMeasurements, *measurements);
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

// bool RemotePersistenceManager::getResultTicket(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::ResultTicket>& ticket)
// {
// 	std::unique_lock<std::mutex> persistenceLock(persistenceMutex);
	
//     if (isDisabled()) return false;
    
//     bool success = false;
//    	STI::TNetwork::TResultTicket_var tResultTicket(new TResultTicket);

// 	try {

// 		success = getTRef()->getResultTicket(convert<STI::Engine::ShotID, STI::TNetwork::TShotID>(sid), tResultTicket);	//remote call

//         if (success) {
//             success &= convert<STI::TNetwork::TResultTicket, std::shared_ptr<ResultTicket>>(tResultTicket.in(), ticket);
//         }
// 	}
// 	catch (CORBA::TRANSIENT&) {
// 	}
// 	catch (CORBA::SystemException&) {
// 	}
// 	catch (CORBA::Exception&) {
// 	}

//     return success && (ticket != 0);
// }

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
