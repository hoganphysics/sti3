#include "TPersistenceManager_i.h"

#include <sti/device/PersistenceManager.h>
#include <sti/engine/ShotResultRecord.h>
#include <sti/engine/EngineJobStatus.h>
#include <sti/engine/Measurement.h>

#include "convert/Convert_EventEngine.h"
#include "convert/Convert_File.h"
#include "convert/Convert_ResultsCollector.h"
#include "convert/Convert_ShotResult.h"
#include "convert/Convert_SequenceResult.h"
#include "TFileServerRefInterface.h"

using STI::TNetwork::TPersistenceManager_i;
using STI::Network::convert;
using ::STI::TNetwork::TShotID;
using STI::Engine::ShotID;
using STI::Engine::ShotResult;
using STI::Engine::ResultsCollector;
using ::STI::TNetwork::TShotResultRecord;
using STI::Engine::ShotResultRecord;
using STI::Engine::FullShotResult;
using ::STI::TNetwork::TSequenceID;
using STI::Engine::SequenceID;
using STI::TNetwork::TSequenceResult;
using STI::Engine::SequenceResult;
using ::STI::TNetwork::TSequenceEntryID;
using STI::Engine::SequenceEntryID;
using ::STI::TNetwork::TEngineJobStatus;
using STI::Engine::EngineJobStatus;
using STI::Network::TFileServerRefInterface;
using STI::Utils::FileID;
using STI::Utils::FileServer;
using STI::Device::ImportedFile;
using STI::Device::ImportFileOptions;
using ::STI::TNetwork::TFileID;
using ::STI::TNetwork::TFileServer;
using ::STI::TNetwork::TFileServer_var;
using ::STI::TNetwork::TImportedFile;
using ::STI::TNetwork::TImportFileOptions;


TPersistenceManager_i::TPersistenceManager_i(const std::shared_ptr<STI::Device::Device>& device)
{
    if (device != 0) {
        device->getPersistenceManager(persistenceManager);        
    }
}

TPersistenceManager_i::~TPersistenceManager_i()
{
}

::CORBA::Boolean TPersistenceManager_i::findShot(const ::STI::TNetwork::TShotID& sid)
{
	bool success = false;

    if (persistenceManager != 0) {

		success = persistenceManager->findShot(convert<TShotID, STI::Engine::ShotID>(sid));
	}

	return success;
}

::CORBA::Boolean TPersistenceManager_i::getParseResult(const ::STI::TNetwork::TParseID& pid, ::STI::TNetwork::TParseResult_out tParseResult)
{
	bool success = false;
	tParseResult = new STI::TNetwork::TParseResult();

    if (persistenceManager != 0) {

		std::shared_ptr<STI::Engine::ParseResult> parseResult;
		STI::TNetwork::TParseResult_var tParseResult_var(new STI::TNetwork::TParseResult);

		success = persistenceManager->getParseResult(convert<TParseID, STI::Engine::ParseID>(pid), parseResult);

		success &= convert<std::shared_ptr<STI::Engine::ParseResult>, TParseResult>(
					parseResult, tParseResult_var);

		(*tParseResult) = tParseResult_var;
	}

	return success;
}

::CORBA::Boolean TPersistenceManager_i::getShotResult(const ::STI::TNetwork::TShotID& sid, ::STI::TNetwork::TShotResult_out tShotResult)
{
	bool success = false;
	tShotResult = new STI::TNetwork::TShotResult();

    if (persistenceManager != 0) {

		std::shared_ptr<STI::Engine::ShotResult> shotResult;
		STI::TNetwork::TShotResult_var tShotResult_var(new STI::TNetwork::TShotResult);

		success = persistenceManager->getShotResult(convert<TShotID, STI::Engine::ShotID>(sid), shotResult);

		success &= convert<std::shared_ptr<STI::Engine::ShotResult>, TShotResult>(
					shotResult, tShotResult_var);

		(*tShotResult) = tShotResult_var;		
	}

	return success;
}

::CORBA::Boolean TPersistenceManager_i::getSequenceResult(const TSequenceID& seqid, ::STI::TNetwork::TSequenceResult_out tSequenceResult)
{
	bool success = false;
	tSequenceResult = new TSequenceResult();

    if (persistenceManager != 0) {

		std::shared_ptr<SequenceResult> sequenceResult;
		STI::TNetwork::TSequenceResult_var tSequenceResult_var(new TSequenceResult);

		success = persistenceManager->getSequenceResult(convert<TSequenceID, SequenceID>(seqid), sequenceResult);

		success &= convert<std::shared_ptr<SequenceResult>, TSequenceResult>(
					sequenceResult, tSequenceResult_var);

		(*tSequenceResult) = tSequenceResult_var;
	}

	return success;
}

::CORBA::Boolean TPersistenceManager_i::saveShot(const ::STI::TNetwork::TShotID& sid, 
											     const ::STI::TNetwork::TFullShotResult& tFullShotResult, ::CORBA::Boolean isOwner)
{
	std::shared_ptr<FullShotResult> fullShotResult;
	bool success = convert<::STI::TNetwork::TFullShotResult, std::shared_ptr<FullShotResult>>(tFullShotResult, fullShotResult);
    
	if (persistenceManager != 0 && success) {
		success &= persistenceManager->saveShot(convert<TShotID, ShotID>(sid), fullShotResult, isOwner);
	}
	else {
		success = false;
	}
	return success;
}


TShotResultRecord* TPersistenceManager_i::transferResults(::STI::TNetwork::TResultsCollector_ptr tResultsCollector)
{
	std::shared_ptr<ResultsCollector> remoteCollector;
    
	ShotResultRecord record;
	STI::TNetwork::TShotResultRecord_var tShotResultRecord(new STI::TNetwork::TShotResultRecord);
	bool success = false;

	if (!CORBA::is_nil(tResultsCollector)) {
		STI::TNetwork::TResultsCollector_var tResultsCollector_var = STI::TNetwork::TResultsCollector::_duplicate(tResultsCollector);
		success = convert<::STI::TNetwork::TResultsCollector_var, std::shared_ptr<ResultsCollector>>(tResultsCollector_var, remoteCollector);
	}

	if (persistenceManager != 0 && success) {

		record = persistenceManager->transferResults(remoteCollector);

		convert<ShotResultRecord, TShotResultRecord>(record, tShotResultRecord);
	}
	else {
		record.recordStatus = STI::Engine::RecordStatus::Error;
	}

	return tShotResultRecord._retn();
}

::CORBA::Boolean TPersistenceManager_i::getMeasurements(const ::STI::TNetwork::TShotID& sid, ::STI::TNetwork::TDeviceIDMeasurementsTupleSeq_out measurements)
{
	bool success = false;
	measurements = new STI::TNetwork::TDeviceIDMeasurementsTupleSeq();

    if (persistenceManager != 0) {

		STI::TNetwork::TDeviceIDMeasurementsTupleSeq_var tDeviceIDMeasurementsTupleSeq_var(new STI::TNetwork::TDeviceIDMeasurementsTupleSeq);
		auto localMeasurements = std::make_shared<STI::Engine::MeasurementMap>();

		success = persistenceManager->getMeasurements(convert<STI::TNetwork::TShotID, STI::Engine::ShotID>(sid), localMeasurements);

		success &= convert<std::shared_ptr<STI::Engine::MeasurementMap>, STI::TNetwork::TDeviceIDMeasurementsTupleSeq>(
			localMeasurements, tDeviceIDMeasurementsTupleSeq_var);
	
		(*measurements) = tDeviceIDMeasurementsTupleSeq_var;
	}

	return success;
}

::STI::TNetwork::TFileServer_ptr TPersistenceManager_i::getFileServer()
{
	std::shared_ptr<STI::Utils::FileServer> fileServer;
	STI::TNetwork::TFileServer_var tFileServer;

	if (persistenceManager != 0 && persistenceManager->getFileServer(fileServer)
		&& TFileServerRefInterface::getTFileServerReference(fileServer, tFileServer)) {
		
		return tFileServer._retn();
	}
	return ::STI::TNetwork::TFileServer::_nil();
}

::CORBA::Boolean TPersistenceManager_i::importFile(
	const ::STI::TNetwork::TFileID& sourceID,
	::STI::TNetwork::TFileServer_ptr sourceServer,
	const ::STI::TNetwork::TImportFileOptions& options,
	::STI::TNetwork::TImportedFile_out importedFile)
{
	bool success = false;
	importedFile = new TImportedFile();

	if (persistenceManager == 0 || CORBA::is_nil(sourceServer)) {
		return false;
	}

	TFileServer_var sourceServerVar = TFileServer::_duplicate(sourceServer);
	std::shared_ptr<FileServer> remoteSourceServer;
	if (!convert<TFileServer_var, std::shared_ptr<FileServer>>(sourceServerVar, remoteSourceServer)) {
		return false;
	}

	auto imported = persistenceManager->importFile(
		convert<TFileID, FileID>(sourceID),
		remoteSourceServer,
		convert<TImportFileOptions, ImportFileOptions>(options));

	if (imported != nullptr) {
		convert<FileID, TFileID>(imported->getFileID(), importedFile->fileID);
		importedFile->importID = CORBA::string_dup(imported->getImportID().c_str());

		std::unique_lock<std::mutex> lock(importedFilesMutex);
		importedFiles[imported->getImportID()] = imported;
		success = true;
	}

	return success;
}

::CORBA::Boolean TPersistenceManager_i::releaseImportedFile(const char* importID)
{
	std::string id = (importID == nullptr) ? "" : importID;
	if (id.empty()) {
		return false;
	}

	std::shared_ptr<ImportedFile> imported;
	{
		std::unique_lock<std::mutex> lock(importedFilesMutex);
		auto it = importedFiles.find(id);
		if (it != importedFiles.end()) {
			imported = it->second;
		}
	}

	if (imported != nullptr) {
		bool success = imported->close();
		if (success) {
			std::unique_lock<std::mutex> lock(importedFilesMutex);
			importedFiles.erase(id);
		}
		return success;
	}

	return persistenceManager != 0 && persistenceManager->releaseImportedFile(id);
}

char* TPersistenceManager_i::getBasePath()
{
	if (persistenceManager != 0) {
		return CORBA::string_dup(persistenceManager->getBasePath().c_str());
	}

	return CORBA::string_dup("");
}

char* TPersistenceManager_i::getTemporaryPath()
{
	if (persistenceManager != 0) {
		return CORBA::string_dup(persistenceManager->getTemporaryPath().c_str());
	}

	return CORBA::string_dup("");
}

void TPersistenceManager_i::addSequence(const ::STI::TNetwork::TSequenceResult& tSequenceResult)
{
	std::shared_ptr<SequenceResult> sequenceResult;
	bool success = convert<::STI::TNetwork::TSequenceResult, std::shared_ptr<SequenceResult>>(tSequenceResult, sequenceResult);
    
	if (persistenceManager != 0 && success) {
		persistenceManager->addSequence(sequenceResult);
	}
}

::CORBA::Boolean TPersistenceManager_i::updateSequence(const TSequenceEntryID& id, const TShotID& shotID, TEngineJobStatus shotStatus, ::CORBA::Boolean isOwner)
{
	bool success;

	if (persistenceManager != 0) {
		success = persistenceManager->updateSequence( 
			convert<TSequenceEntryID, SequenceEntryID>(id), 
			convert<TShotID, ShotID>(shotID), 
			convert<TEngineJobStatus, EngineJobStatus>(shotStatus), 
			isOwner);
	}
	else {
		success = false;
	}
	return success;
}

::CORBA::Boolean TPersistenceManager_i::saveSequence(const ::STI::TNetwork::TSequenceResult& tSequenceResult, ::CORBA::Boolean isOwner)
{
	std::shared_ptr<SequenceResult> sequenceResult;
	bool success = convert<::STI::TNetwork::TSequenceResult, std::shared_ptr<SequenceResult>>(tSequenceResult, sequenceResult);
    
	if (persistenceManager != 0 && success) {
		success &= persistenceManager->saveSequence(sequenceResult, isOwner);
	}
	else {
		success = false;
	}
	return success;
}

::CORBA::Boolean TPersistenceManager_i::ping()
{
    return true;
}
