#include "LocalPersistenceManager.h"

#include <sti/device/DeviceID.h>

#include <sti/engine/EventEngineJob.h>
#include <sti/engine/FullShotResult.h>
#include <sti/engine/ParseResult.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/ShotResult.h>
#include <sti/engine/StackTraceResult.h>

#include "EventEngine.h"
#include "EventEngineDependencyTree.h"
#include "LocalResultsCollector.h"
#include "ResultsDocumenter.h"
#include "SerializedRepository.h"
#include "StackTraceData.h"
#include "TransientRepository.h"

#include <filesystem>
namespace fs = std::filesystem;


using STI::Device::LocalPersistenceManager;

using STI::Engine::ResultsCollector;
using STI::Engine::LocalResultsCollector;
using STI::Device::DeviceID;
using STI::Engine::EventEngineDependencyTree;
using STI::Engine::ResultsDocumenter;
using STI::Engine::ResultsPaths;
using STI::Engine::ShotRepository;
using STI::Engine::ResultTicket;
using STI::Engine::ShotID;
using STI::Engine::ShotResult;
using STI::Engine::SerializedRepository;
using STI::Engine::TransientRepository;
using STI::Engine::ShotResultRecord;
using STI::Engine::FullShotResult;
using STI::Engine::ParseResult;


LocalPersistenceManager::LocalPersistenceManager(const DeviceID& deviceID, const std::string& basePath, 
        const std::shared_ptr<STI::Utils::FileHolderFactory>& fileHolderFactory,
        const std::shared_ptr<STI::Device::DeviceCollection>& collection)
: localDeviceID(deviceID), fileHolderFactory(fileHolderFactory), resultBuffer(5), deviceCollection(collection)
{
    defaultRepository = std::make_shared<SerializedRepository>(basePath);
    transientRepository = std::make_shared<TransientRepository>(basePath);
}

LocalPersistenceManager::~LocalPersistenceManager()
{
    //serialize all shots in memory
}

std::string LocalPersistenceManager::makeBasePath(const std::string& rootPath, const DeviceID& deviceID)
{
    std::filesystem::path root(rootPath);

    if (!std::filesystem::exists(root)) {
        std::filesystem::create_directory(root);
    }

    auto devicePath = root / deviceID.getID();

    if (!std::filesystem::exists(devicePath)) {
        std::filesystem::create_directories(devicePath);
    }

    return devicePath.string();
}

void LocalPersistenceManager::setFileHolderFactory(const std::shared_ptr<STI::Utils::FileHolderFactory>& factory)
{
    fileHolderFactory = factory;
}

std::shared_ptr<STI::Utils::FileHolder> LocalPersistenceManager::makeFileHolder(const std::string& filename)
{
    if (fileHolderFactory != 0) {
        return fileHolderFactory->makeFileHolder(filename);
    }

    std::shared_ptr<STI::Utils::FileHolder> nullFile;
    return nullFile;
}

void LocalPersistenceManager::setShotRepository(const std::shared_ptr<STI::Engine::ShotRepository>& repo)
{
    shotRepository = repo;
}

bool LocalPersistenceManager::getShotRepository(std::shared_ptr<STI::Engine::ShotRepository>& repo)
{
    if (shotRepository != 0) {
        repo = shotRepository;        
    }
    else {
        repo = defaultRepository;
    }

    return (repo != 0);
}

ShotResultRecord LocalPersistenceManager::transferResults(const std::shared_ptr<STI::Engine::ResultsCollector>& resultsCollector)
{
    ShotResultRecord record(localDeviceID);

    if (resultsCollector == 0) {
        record.recordStatus = STI::Engine::RecordStatus::Error;
        return record;
    }

    std::shared_ptr<ShotResult> shotResult;
    
    if (!getShotLocal(resultsCollector->getShotID(), shotResult)) {
        //Shot not found. This means the data from the local device is not available, and 
        //there is no record of the list of dependent devices that this device owned for this shot.
        record.recordStatus = STI::Engine::RecordStatus::MissingResults;
        return record;
    }

    return transferResults(resultsCollector, shotResult, true);
}


ShotResultRecord LocalPersistenceManager::transferResults(const std::shared_ptr<ResultsCollector>& resultsCollector, 
                                              const std::shared_ptr<ShotResult>& shotResult, bool transferDependents)
{
    ShotResultRecord record(localDeviceID);
    
    if (resultsCollector == 0 || shotResult == 0) {
        record.recordStatus = STI::Engine::RecordStatus::Error;
        return record;
    }

    bool success = true;

    //Attributes
    for (auto& attribs : shotResult->attributes) {
        
        success &= resultsCollector->addAttributes(attribs.first, attribs.second);
    }

    //Measurements
    success &= resultsCollector->addMeasurements(shotResult->measurements);


    if (success) {
        //resultsCollector->markRecord(shotResult->);
        record.recordStatus = STI::Engine::RecordStatus::Complete;
    }
    else {
        record.recordStatus = STI::Engine::RecordStatus::Error;
    }

	std::shared_ptr<STI::Device::Device> device;
    std::shared_ptr<PersistenceManager> persistenceManager;

    for (auto& dependency : shotResult->shotResultRecord.dependencies) {

        ShotResultRecord depRecord(dependency.deviceID);

        if (transferDependents) {

            if (deviceCollection->get(dependency.deviceID, device) && device != 0 
                && device->getPersistenceManager(persistenceManager)) {
                
                depRecord = persistenceManager->transferResults(resultsCollector);

            }
            else {
                depRecord.recordStatus = STI::Engine::RecordStatus::MissingDevice;
            }
        }

        record.dependencies.push_back(depRecord);
    }

    return record;

}

bool LocalPersistenceManager::getShotLocal(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::ShotResult>& result)
{
    std::shared_ptr<FullShotResult> fullResult;
    if (resultBuffer.get(sid, fullResult) && fullResult != 0 ) {
        result = fullResult->shotResult;
        return true;
    }

    if (transientRepository->getShotResult(sid, result)) {
        return true;
    }

    bool success = false;
    std::shared_ptr<STI::Engine::ShotRepository> repo;

    if (getShotRepository(repo)) {
        success = repo->getShotResult(sid, result);
    }

    if (!success) {
        success = defaultRepository->getShotResult(sid, result);
    }

    return success;
}

bool LocalPersistenceManager::getParseResult(const STI::Engine::ParseID& pid, std::shared_ptr<STI::Engine::ParseResult>& parseResult)
{

    std::shared_ptr<PersistenceManager> delegate;
    bool hasDelegate = false;

    if (hasDelegate) {
        if(delegate->getParseResult(pid, parseResult)) {
            return true;
        }
    }

    return false;
}

bool LocalPersistenceManager::getShotResult(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::ShotResult>& result)
{
    std::shared_ptr<PersistenceManager> delegate;
    bool hasDelegate = false;

    if (hasDelegate) {
        if(delegate->getShotResult(sid, result)) {
            return true;
        }
    }

    return getShotLocal(sid, result);
}

bool LocalPersistenceManager::getMeasurements(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::MeasurementVector>& measurements)
{
    std::shared_ptr<STI::Engine::ShotResult> shotResult;
    if (getShotResult(sid, shotResult) && shotResult != 0) {
        measurements = shotResult->measurements;
        return (measurements != 0);
    }
    return false;
}

bool LocalPersistenceManager::addToBuffer(const std::shared_ptr<FullShotResult>& fullShotResult)
{
    std::shared_ptr<FullShotResult> bufferedResult;
    
    if (resultBuffer.addAndRemove(fullShotResult->shotResult->sid, fullShotResult, bufferedResult)) {
        //The buffer was full. Need to save old bufferedResult to disk;

        return saveShotLocal(bufferedResult->shotResult->sid, bufferedResult, false);
    }

    return false;
}

bool LocalPersistenceManager::saveShot(const STI::Engine::ShotID& sid, 
                                        const std::shared_ptr<FullShotResult>& fullShotResult, bool isOwner)
{
    std::shared_ptr<PersistenceManager> delegate;
    bool hasDelegate = false;

    if (hasDelegate && isOwner && delegate != 0) {
        if(delegate->saveShot(sid, fullShotResult, true)) {     //transfer ownership to delegate
            return true;
        }
        //if failed, save locally
    }

    if (isOwner) {
        
        // if (sid.parseID.shotType == STI::Engine::ParseID::ShotType::SingleUndocumented) {
        //     return addToBuffer(shotResult);
        // }

        return saveShotLocal(sid, fullShotResult, isOwner);
    }
    
    return addToBuffer(fullShotResult);
        



    // std::shared_ptr<ResultsCollector> collector;

    // if (sid.parseID.shotType == STI::Engine::ParseID::ShotType::SingleUndocumented) {
    //     //Skip documentation
    //     return true;
    // }

    // std::set<unsigned> priorities;
    // delegatePriorities.getKeys(priorities);

    // std::vector<std::shared_ptr<PersistenceManager>> orderedDelegates;

    // for (auto& p : priorities) {
    //     DeviceID id;
    //     delegatePriorities.get(p, id);

    //     if (delegates.get(id, delegate) && delegate != 0) {
    //         orderedDelegates.push_back(delegate);
    //     }
    // }

    // if (orderedDelegates.size() > 0) {
    //     return orderedDelegates.front()->saveShot(sid, eventEngine);
    //     //other delegates?
    // }
    // else {
    //     return saveShotLocal(sid, eventEngine);
    // }

    // return false;
}

bool isPartial(const std::shared_ptr<ShotResult>& shotResult)
{
    return false;
}



//replace resultsDocumenter with localDocumenter and make it a function argument.
//allow engine to call saveShotLocal with a TransientResultsDocumenter which auto deletes itself
//after going out of scope. Data is copied from this documenter into a MixedValue when read() is called.
bool LocalPersistenceManager::saveShotLocal(const STI::Engine::ShotID& sid, 
                                            const std::shared_ptr<FullShotResult>& fullShotResult, bool isOwner)
{

    //bool isShotOwner = sid.parseID.jobSourceID == localDeviceID;

    std::shared_ptr<STI::Engine::ShotRepository> repo;
    if (!getShotRepository(repo)) return false;

    if (sid.parseID.shotConfig.shotType == STI::Engine::ShotType::SingleUndocumented) {
        repo = transientRepository;
    }

    // std::shared_ptr<ShotResult> finalShotResult = shotResult;

    //happens on (remote) delegate, where data should be saved.
    ResultsPaths resultsPaths = repo->preparePaths(sid);    //e.g., make directory sturcture
    auto collector = std::make_shared<LocalResultsCollector>(sid, resultsPaths, fileHolderFactory);

    auto shotRecord = transferResults(collector, fullShotResult->shotResult, isOwner);   //collector is passed on to all devices in shot
    collector->setRecord(shotRecord);

    transferParseResult(fullShotResult->parseResult, resultsPaths.timingPath);

    auto completeResult = std::make_shared<FullShotResult>();  
    completeResult->parseResult = fullShotResult->parseResult;
    completeResult->shotResult = collector->getResults();

    bool success = repo->saveShot(sid, completeResult);

    if (!success) {
        success = defaultRepository->saveShot(sid, completeResult);
    }

    if (isOwner && isPartial(fullShotResult->shotResult)) {
        //add to list of partial shots; need to attempt to transfer this result again later
    }

    return success;


    // if (resultsDocumenter == 0) return false;


    
    // ResultsPaths resultsPaths = resultsDocumenter->preparePaths(sid);    //e.g., make directory sturcture

    // collector = std::make_shared<LocalResultsCollector>(sid, eventEngine->getParsedTree(), 
    //                                                     resultsPaths, fileHolderFactory);

    // eventEngine->transferResults(collector);   //collector is passed on to all devices in shot

    // return resultsDocumenter->save(resultsPaths, collector);
}



void LocalPersistenceManager::transferParseResult(std::shared_ptr<ParseResult> parseResult, const std::string& timingPath)
{
    if (parseResult == 0 || parseResult->stackTraceResult == 0 || parseResult->stackTraceResult->stackTraceData == 0) return;

    auto stackTraceData = parseResult->stackTraceResult->stackTraceData;
    auto files = stackTraceData->getTimingFiles();

    for (auto& file : files) {
        if (file != 0) {
            auto inputFilename = file->getFilename();
            fs::path inputPath = inputFilename;

            fs::path localPath = timingPath;
            localPath /= inputPath.filename();

            auto localFileHandle = fileHolderFactory->makeFileHolder( STI::Utils::makeUniquePath( localPath.string() ) );

            file->transferFile(localFileHandle);    //transfer file to local
            stackTraceData->replaceFile(inputFilename, localFileHandle);            
        }
    }
}

void LocalPersistenceManager::addPersistenceDelegate(const DeviceID& id, 
                                                     const std::shared_ptr<PersistenceManager>& manager, unsigned priority)
{
    if (!delegates.add(id, manager)) return;

    //ensure unique priority
    unsigned p = priority;
    while (delegatePriorities.contains(p)) {
        p++;
    }

    delegatePriorities.add(p, id);
}

void LocalPersistenceManager::removePersistenceDelegate(const DeviceID& id)
{
    delegates.remove(id);

    std::set<unsigned> priorities;
    delegatePriorities.getKeys(priorities);

    DeviceID priorityID;

    for (auto& p : priorities) {
        if (delegatePriorities.get(p, priorityID) && priorityID == id) {
            delegatePriorities.remove(p);
        }
    }
}

