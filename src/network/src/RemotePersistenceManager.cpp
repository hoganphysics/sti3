#include "RemotePersistenceManager.h"

#include "convert/Convert_ResultsCollector.h"
#include "convert/Convert_EventEngine.h"
#include "convert/Convert_ShotResult.h"
#include "convert/Convert_SequenceResult.h"
#include "convert/Convert_File.h"
#include "generated/orbTypes.h"
#include "NetworkFileHolder.h"
#include "NetworkFileServer.h"
#include "NetworkResultsCollector.h"
#include <sti/engine/RawEvent.h>


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
using STI::TNetwork::TParseResult;
using STI::Engine::ParseResult;
using STI::Engine::FullShotResult;
using STI::TNetwork::TFullShotResult;
using STI::Engine::SequenceID;
using STI::Engine::SequenceResult;
using STI::TNetwork::TSequenceResult;
using STI::Engine::SequenceResult;
using STI::Engine::SequenceEntryID;
using ::STI::TNetwork::TSequenceEntryID;
using STI::Engine::EngineJobStatus;
using ::STI::TNetwork::TEngineJobStatus;
using ::STI::TNetwork::TFileServer_var;


RemotePersistenceManager::RemotePersistenceManager(::STI::TNetwork::TPersistenceManager_var manager)
: TReferenceHolder<TPersistenceManager>(manager)
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


bool RemotePersistenceManager::getParseResult(const STI::Engine::ParseID& pid, std::shared_ptr<STI::Engine::ParseResult>& parseResult)
{
	std::unique_lock<std::mutex> persistenceLock(persistenceMutex);

	if (isDisabled()) return false;

	STI::TNetwork::TParseResult_var tParseResult(new STI::TNetwork::TParseResult);

    bool success = false;

	try {
		success = getTRef()->getParseResult(
					convert<STI::Engine::ParseID, STI::TNetwork::TParseID>(pid), 
					tParseResult);	//remote call

		convert<TParseResult, std::shared_ptr<ParseResult>>(tParseResult, parseResult);
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}

    return success;
}

bool RemotePersistenceManager::getShotResult(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::ShotResult>& shotResult)
{
	std::unique_lock<std::mutex> persistenceLock(persistenceMutex);

	if (isDisabled()) return false;

	STI::TNetwork::TShotResult_var tShotResult(new STI::TNetwork::TShotResult);

    bool success = false;

	try {
		success = getTRef()->getShotResult(
					convert<STI::Engine::ShotID, STI::TNetwork::TShotID>(sid), 
					tShotResult);	//remote call

		convert<TShotResult, std::shared_ptr<ShotResult>>(tShotResult, shotResult);
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}

    return success && (shotResult != 0);
}


bool RemotePersistenceManager::getSequenceResult(const SequenceID& id, std::shared_ptr<SequenceResult>& sequenceResult)
{
	std::unique_lock<std::mutex> persistenceLock(persistenceMutex);

	if (isDisabled()) return false;

	STI::TNetwork::TSequenceResult_var tSequenceResult(new STI::TNetwork::TSequenceResult);

    bool success = false;

	try {
		success = getTRef()->getSequenceResult(
					convert<STI::Engine::SequenceID, STI::TNetwork::TSequenceID>(id), 
					tSequenceResult);	//remote call

		convert<TSequenceResult, std::shared_ptr<SequenceResult>>(tSequenceResult, sequenceResult);
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}

    return success && (sequenceResult != 0);
}

bool RemotePersistenceManager::saveShot(const STI::Engine::ShotID& sid, const std::shared_ptr<FullShotResult>& fullShotResult, bool isOwner)
{
	std::unique_lock<std::mutex> persistenceLock(persistenceMutex);
	
    if (isDisabled()) return false;
    
    bool success = false;
   	STI::TNetwork::TFullShotResult tFullShotResult;

	try {
        success = convert<std::shared_ptr<FullShotResult>, TFullShotResult>(fullShotResult, tFullShotResult);

        if (success) {
    		success = getTRef()->saveShot(
						convert<STI::Engine::ShotID, STI::TNetwork::TShotID>(sid), 
						tFullShotResult,
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

bool RemotePersistenceManager::getMeasurements(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::MeasurementMap>& measurements)
{
	std::unique_lock<std::mutex> persistenceLock(persistenceMutex);

	if (isDisabled()) return false;

	STI::TNetwork::TDeviceIDMeasurementsTupleSeq_var tDeviceIDMeasurementsTupleSeq_var(new STI::TNetwork::TDeviceIDMeasurementsTupleSeq);

	measurements = std::make_shared<STI::Engine::MeasurementMap>();

	bool success = false;

	try {
		success = getTRef()->getMeasurements(convert<ShotID, TShotID>(sid), tDeviceIDMeasurementsTupleSeq_var);	//remote call
 
		success &= convert<::STI::TNetwork::TDeviceIDMeasurementsTupleSeq, std::shared_ptr<STI::Engine::MeasurementMap>>(
			tDeviceIDMeasurementsTupleSeq_var, measurements);
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

void RemotePersistenceManager::setFileHolderFactory(const std::shared_ptr<STI::Utils::FileHolderFactory>& factory)
{
	//not allowed (FilHolderFactory can only be set locally)
}

std::shared_ptr<STI::Utils::FileHolder> RemotePersistenceManager::makeFileHolder(const std::string& path, const std::string& filename)
{
	std::shared_ptr<STI::Utils::FileHolder> fileHolder;	//null
	return fileHolder;
}

std::shared_ptr<STI::Utils::FileHolder> RemotePersistenceManager::makeVirtualFileHolder(const STI::Utils::FileID& fileID)
{
	std::shared_ptr<STI::Utils::FileHolder> fileHolder; //null
	return fileHolder;
}

std::shared_ptr<STI::Utils::VirtualFileServer> RemotePersistenceManager::makeVirtualFileServer()
{
	auto fileServer = std::shared_ptr<STI::Network::NetworkVirtualFileServer>();
	return fileServer;
}

bool RemotePersistenceManager::getFileServer(std::shared_ptr<STI::Utils::FileServer>& server) 
{
	std::unique_lock<std::mutex> persistenceLock(persistenceMutex);

	if (isDisabled()) return false;

    bool success = false;

	try {
		TFileServer_var tFileServer = getTRef()->getFileServer();	//remote call
		success = convert<TFileServer_var, std::shared_ptr<STI::Utils::FileServer>>(tFileServer, server);
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}

    return success;
}

void RemotePersistenceManager::addSequence(const std::shared_ptr<SequenceResult>& sequenceResult)
{
	std::unique_lock<std::mutex> persistenceLock(persistenceMutex);
	
    if (isDisabled()) return;
    
    bool success = false;
   	STI::TNetwork::TSequenceResult tSequenceResult;

	try {
        success = convert<std::shared_ptr<SequenceResult>, TSequenceResult>(sequenceResult, tSequenceResult);

        if (success) {
    		getTRef()->addSequence(tSequenceResult);	//remote call
        }
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}
}

bool RemotePersistenceManager::updateSequence(const SequenceEntryID& id, const ShotID& shotID, const EngineJobStatus& shotStatus, bool isOwner)
{
	std::unique_lock<std::mutex> persistenceLock(persistenceMutex);
	
    if (isDisabled()) return false;
    
    bool success = false;
   	STI::TNetwork::TFullShotResult tFullShotResult;

	try {
        if (success) {
    		success = getTRef()->updateSequence(
						convert<SequenceEntryID, TSequenceEntryID>(id), 
						convert<STI::Engine::ShotID, STI::TNetwork::TShotID>(shotID), 
						convert<EngineJobStatus, TEngineJobStatus>(shotStatus), 
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

bool RemotePersistenceManager::saveSequence(const std::shared_ptr<SequenceResult>& sequenceResult, bool isOwner)
{
	std::unique_lock<std::mutex> persistenceLock(persistenceMutex);
	
    if (isDisabled()) return false;
    
    bool success = false;
   	STI::TNetwork::TSequenceResult tSequenceResult;

	try {
        success &= convert<std::shared_ptr<SequenceResult>, TSequenceResult>(sequenceResult, tSequenceResult);

        if (success) {
    		success = getTRef()->saveSequence(
						tSequenceResult,
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
