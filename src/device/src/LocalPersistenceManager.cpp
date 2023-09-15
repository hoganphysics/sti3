#include "LocalPersistenceManager.h"

#include <sti/utils/Configuration.h>
#include <sti/device/DeviceID.h>

#include <sti/engine/EventEngineJob.h>
#include <sti/engine/EventEngineScheduler.h>
#include <sti/engine/FullShotResult.h>
#include <sti/engine/ParseResult.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/ShotResult.h>
#include <sti/engine/StackTraceData.h>
#include <sti/engine/StackTraceResult.h>

#include <sti/utils/Image.h>
#include <sti/utils/FileServer.h>

#include "EventEngine.h"
#include "EventEngineDependencyTree.h"
#include "LocalResultsCollector.h"
#include "LocalResultsCollectorFactory.h"
#include "PersistenceTarget.h"
#include "PersistenceTargetHolder.h"
#include "ResultsDocumenter.h"
#include "SerializedRepository.h"
#include "TransientRepository.h"
#include "LocalFileServer.h"


#include <filesystem>
namespace fs = std::filesystem;


using STI::Device::LocalPersistenceManager;

using STI::Device::PersistenceTarget;
using STI::Device::PersistenceTargetHolder;
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
using STI::Utils::Configuration;
using STI::Engine::ShotType;


LocalPersistenceManager::LocalPersistenceManager(const DeviceID& deviceID, const Configuration& config, const std::string& basePath, 
        const std::shared_ptr<STI::Utils::FileHolderFactory>& fileHolderFactory,
        const std::shared_ptr<STI::Device::DeviceCollection>& collection)
: localDeviceID(deviceID), 
fileHolderFactory(fileHolderFactory), 
resultBuffer( config.get<int>("PersistenceManager", "resultBufferSize", 5) ), 
sequenceBuffer( config.get<int>("PersistenceManager", "sequenceBufferSize", 5) ), 
deviceCollection(collection),
basePath(basePath)
{
    auto server = std::make_shared<STI::Utils::LocalFileServer>(deviceID);
    setFileServer(server);

    defaultRepository = std::make_shared<SerializedRepository>(basePath);
    transientRepository = std::make_shared<TransientRepository>(basePath, server);

    auto resultsCollectionFactory = std::make_shared<STI::Engine::LocalResultsCollectorFactory>();
    setResultsCollectorFactory(resultsCollectionFactory);

    virtualFileServerFactory = std::make_shared<STI::Utils::LocalVirtualFileServerFactory>();
}

LocalPersistenceManager::~LocalPersistenceManager()
{
    //serialize all shots in memory

    //save all persistence targets
    for (auto& holder : persistenceTargetHolders) {
        if (holder != 0) {
            holder->save();
        }
    }
}

void LocalPersistenceManager::attachEngineScheduler(const std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler)
{
    eventEngineScheduler = scheduler;   //weak_ptr to avoid circular reference...
}

void LocalPersistenceManager::addPersistenceTarget(const std::shared_ptr<PersistenceTarget>& target)
{
    if (target != 0) {
        // std::filesystem::path filename(getBasePath());
        // filename /= (target->getFilenameStem() + ".ini");

        auto holder = std::make_shared<PersistenceTargetHolder>(target, getBasePath());
        persistenceTargetHolders.push_back(holder);
    }
}

void LocalPersistenceManager::loadPersistenceTargets()
{
    for (auto& holder : persistenceTargetHolders) {
        if (holder != 0) {
            holder->load();
        }
    }
}

std::string LocalPersistenceManager::makeBasePath(const std::string& rootPath, const std::string& deviceID, bool autocreate)
{
    std::filesystem::path root(rootPath);

    if (!std::filesystem::exists(root) && autocreate) {
        std::filesystem::create_directory(root);

    }

    std::string forbidden = "<>:\"\\|?*";
    auto devicePath = root / STI::Utils::replaceChars(deviceID, forbidden, "_");

    if (!std::filesystem::exists(devicePath) && autocreate) {
        std::filesystem::create_directories(devicePath);
    }

    return devicePath.string();
}

std::string LocalPersistenceManager::getBasePath() const
{
    //basePath = .sti/address/module/name
    return basePath;
}

bool LocalPersistenceManager::getLogBasePath(const STI::Utils::TimeStamp& timestamp, std::string& logBasePath)
{
    //for date in timestamp

    std::shared_ptr<STI::Engine::ShotRepository> repo;
    if (!getShotRepository(repo)) return false;

    logBasePath = repo->prepareLogPath(timestamp, false);   //don't make path if it doesn't exist
    return true;
}

bool LocalPersistenceManager::getLogBasePath(const STI::Utils::TimeStamp& timestamp, const DeviceID& deviceID, std::string& logBasePath)
{
    //for date in timestamp
    std::string todaysLogPath;
    if (!getLogBasePath(timestamp, todaysLogPath)) return false;

    logBasePath = makeBasePath(todaysLogPath, deviceID.getID(), false);   //don't make path if it doesn't exist

    return false;
}

bool LocalPersistenceManager::makeLogPath(const STI::Utils::TimeStamp& timestamp, std::string& logPath)
{
    std::shared_ptr<STI::Engine::ShotRepository> repo;
    if (!getShotRepository(repo)) return false;

    // basePath/logs/year/month/day/
    logPath = repo->prepareLogPath(timestamp, true);    //make directory sturcture
    return true;
}

bool LocalPersistenceManager::makeLogPath(const STI::Utils::TimeStamp& timestamp, const DeviceID& deviceID, std::string& logPath)
{
    std::string todaysLogPath;  // basePath/logs/year/month/day/
    if (!makeLogPath(timestamp, todaysLogPath)) return false;

    logPath = makeBasePath(todaysLogPath, deviceID.getID());     // todaysLogPath/device_id
    return true;
}

void LocalPersistenceManager::setFileHolderFactory(const std::shared_ptr<STI::Utils::FileHolderFactory>& factory)
{
    fileHolderFactory = factory;
}

void LocalPersistenceManager::setVirtualFileServerFactory(const std::shared_ptr<STI::Utils::VirtualFileServerFactory>& factory)
{
    virtualFileServerFactory = factory;
}

void LocalPersistenceManager::setResultsCollectorFactory(const std::shared_ptr<STI::Engine::ResultsCollectorFactory>& factory)
{
    resultsCollectorFactory = factory;
}

bool LocalPersistenceManager::getFileServer(std::shared_ptr<STI::Utils::FileServer>& server)
{
    server = fileServer;
    return (server != 0);
}

void LocalPersistenceManager::setFileServer(const std::shared_ptr<STI::Utils::FileServer>& server)
{
    fileServer = server;
}

std::shared_ptr<STI::Utils::VirtualFileServer> LocalPersistenceManager::makeVirtualFileServer()
{
    if (virtualFileServerFactory == 0) {
        std::shared_ptr<STI::Utils::VirtualFileServer> empty = std::make_shared<STI::Utils::VirtualFileServer>();
        return empty;
    }
    return virtualFileServerFactory->makeVirtualFileServer();
}

std::shared_ptr<STI::Utils::FileHolder> LocalPersistenceManager::makeFileHolder(const std::string& path, const std::string& filename)
{
    if (fileHolderFactory != 0) {
        return fileHolderFactory->makeFileHolder(path, filename);
    }

    std::shared_ptr<STI::Utils::FileHolder> nullFile;
    return nullFile;
}

std::shared_ptr<STI::Utils::FileHolder> LocalPersistenceManager::makeVirtualFileHolder(const STI::Utils::FileID& fileID)
{
    if (fileHolderFactory != 0) {
        return fileHolderFactory->makeVirtualFileHolder(fileID);
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

    std::shared_ptr<STI::Utils::FileServer> fs;
    std::shared_ptr<STI::Utils::VirtualFileServer> measurementFileServer;

    //Measurements
    if (shotResult->measurements != 0) {
        for (auto& tuple : *shotResult->measurements) {  //tuple = {DeviceID, MeasurementVector}

            if (tuple.second.size() > 0) {

                if (tuple.second.front() != 0 && tuple.second.front()->getFileServer(measurementFileServer)) {
                    fs = measurementFileServer;
                }
                else {
                    fs = fileServer;    //default in case Measurement is missing VirtualFileServer
                }
                success &= resultsCollector->addMeasurements(tuple.first, tuple.second, fs);
            }
        }        
    }

    if (success) {
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

bool LocalPersistenceManager::findShotLocal(const STI::Engine::ShotID& sid)
{
    if (resultBuffer.contains(sid)) {
        return true;
    }

    if (transientRepository->findShotResult(sid)) {
        return true;
    }

    bool success = false;
    std::shared_ptr<STI::Engine::ShotRepository> repo;

    if (getShotRepository(repo)) {
        success = repo->findShotResult(sid);
    }

    if (!success) {
        success = defaultRepository->findShotResult(sid);
    }

    return success;
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

bool LocalPersistenceManager::findBufferedParseResult(const STI::Engine::ParseID& pid, STI::Engine::ShotID& sid)
{
    std::set<STI::Engine::ShotID> shotIDs;
    resultBuffer.getKeys(shotIDs);
    bool found = false;

    for (auto& shotID : shotIDs) {
        if (shotID.parseID == pid) {
            sid = shotID;
            found = true;
            break;
        }
    }
    return found;
}

bool LocalPersistenceManager::getParseResultLocal(const STI::Engine::ParseID& pid, std::shared_ptr<STI::Engine::ParseResult>& parseResult)
{
    std::shared_ptr<FullShotResult> fullResult;
    STI::Engine::ShotID sid;

    //search in shot buffer (recent shots might be associated with parseResult)
    if (findBufferedParseResult(pid, sid) && resultBuffer.get(sid, fullResult) && fullResult != 0) {
        parseResult = fullResult->parseResult;
        return true;
    }

    if (transientRepository->getParseResult(pid, parseResult)) {
        return true;
    }

    bool success = false;
    std::shared_ptr<STI::Engine::ShotRepository> repo;

    if (getShotRepository(repo)) {
        success = repo->getParseResult(pid, parseResult);
    }

    if (!success) {
        success = defaultRepository->getParseResult(pid, parseResult);
    }

    return success;
}


bool LocalPersistenceManager::findShot(const STI::Engine::ShotID& sid)
{
    std::shared_ptr<PersistenceManager> delegate;
    bool hasDelegate = false;

    if (hasDelegate && delegate != 0) {
        if(delegate->findShot(sid)) {
            return true;
        }
    }

    return findShotLocal(sid);
}

bool LocalPersistenceManager::getParseResult(const STI::Engine::ParseID& pid, std::shared_ptr<STI::Engine::ParseResult>& parseResult)
{
    std::shared_ptr<PersistenceManager> delegate;
    bool hasDelegate = false;

    if (hasDelegate && delegate != 0) {
        if(delegate->getParseResult(pid, parseResult)) {
            return true;
        }
    }

    if (auto observe = eventEngineScheduler.lock()) {
        if (observe->getParseResult(pid, parseResult)) {
            return true;
        }
    }

    if (getParseResultLocal(pid, parseResult) && parseResult != 0) {
        //rebind FileHolders of retrieved ParseResult so they match the PersistenceManager
        //(ensures that they will be served to the network properly)
        // if (parseResult->stackTraceResult != 0 && parseResult->stackTraceResult->stackTraceData != 0) {
        //     parseResult->stackTraceResult->stackTraceData->setFileHolderFactory(fileHolderFactory);
        // }
        return true;
    }

    return false;
}

bool LocalPersistenceManager::getShotResult(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::ShotResult>& result)
{
    std::shared_ptr<PersistenceManager> delegate;
    bool hasDelegate = false;

    if (hasDelegate && delegate != 0) {
        if(delegate->getShotResult(sid, result)) {
            return true;
        }
    }

    return getShotLocal(sid, result);
}

bool LocalPersistenceManager::getMeasurements(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::MeasurementMap>& measurements)
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
    if (fullShotResult == 0) return false;
    if (fullShotResult->shotResult == 0) return false;

    std::shared_ptr<FullShotResult> bufferedResult;
    if (resultBuffer.addAndRemove(fullShotResult->shotResult->sid, fullShotResult, bufferedResult)) {
        //The buffer was full. Need to save old bufferedResult to disk;
        return saveShotLocal(bufferedResult->shotResult->sid, bufferedResult, false);
    }

    return resultBuffer.contains(fullShotResult->shotResult->sid);;
}

bool LocalPersistenceManager::addToBuffer(const std::shared_ptr<STI::Engine::SequenceResult>& sequenceResult)
{
    if (sequenceResult == 0) return false;

    std::shared_ptr<STI::Engine::SequenceResult> bufferedResult;
    if (sequenceBuffer.addAndRemove(sequenceResult->seqid, sequenceResult, bufferedResult)) {
        //The buffer was full. Need to save old bufferedResult to disk;
        return saveSequenceLocal(bufferedResult, false);
    }
    return sequenceBuffer.contains(sequenceResult->seqid);
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

    std::shared_ptr<STI::Engine::SequenceResult> sequenceResult;
    if (sid.parseID.shotConfig.shotType == ShotType::Sequence && 
        getSequenceResult(sid.parseID.sequenceEntryID.seqID, sequenceResult)) {
        
        if (!updateSequence(sid.parseID.sequenceEntryID, sid, STI::Engine::EngineJobStatus::Completed, isOwner)) {
            saveSequence(sequenceResult, isOwner);
        }
        // sequenceResult->status[sid.parseID.sequenceEntryID.seqIndex] = STI::Engine::EngineJobStatus::Completed;
    }

    if (isOwner) {
        
        // if (sid.parseID.shotType == STI::Engine::ParseID::ShotType::SingleUndocumented) {
        //     return addToBuffer(shotResult);
        // }

        return saveShotLocal(sid, fullShotResult, isOwner);
    }

    //not owner; save Images with custom ImageWritter
    ResultsPaths resultsPaths;
    getResultsPaths(sid, resultsPaths);
    resultsPaths.dataPath;

    //if any measurements are Images with a custom writter, write to disk now before transferring
    std::shared_ptr<STI::Utils::ImageWriter> dummyWriter;     //hack; use null writter so only Images with a custom writter will be written

    if (fullShotResult != 0 && fullShotResult->shotResult != 0 && fullShotResult->shotResult->measurements != 0) {
        for (auto& tuple : *(fullShotResult->shotResult->measurements)) {
            for (auto& m : tuple.second) {
                if (m != 0 && m->data().isType(STI::Utils::MixedValueType::Image)) {
                    m->data().getImage()->writeToFile(dummyWriter, resultsPaths.dataPath);
                }
            }
        }        
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

bool LocalPersistenceManager::updateSequence(const STI::Engine::SequenceEntryID& id, const STI::Engine::ShotID& shotID, 
                                             const STI::Engine::EngineJobStatus& shotStatus, bool isOwner)
{
    std::shared_ptr<PersistenceManager> delegate;
    bool hasDelegate = false;

    if (hasDelegate && isOwner && delegate != 0) {
        if(delegate->updateSequence(id, shotID, shotStatus, true)) {     //transfer ownership to delegate
            return true;
        }
        //if failed, save locally
    }

    return updateSequenceLocal(id, shotID, shotStatus, isOwner);
}

bool LocalPersistenceManager::updateSequenceLocal(const STI::Engine::SequenceEntryID& id, const STI::Engine::ShotID& shotID, 
                                             const STI::Engine::EngineJobStatus& shotStatus, bool isOwner)
{
    std::shared_ptr<STI::Engine::ShotRepository> repo;
    if (!getShotRepository(repo)) return false;

    //this should be called when the shot is saved
    if (isOwner && repo->findSequenceResult(id.seqID)) {
        return repo->updateSequence(id, shotID, shotStatus);
    }

    //update buffered result
    std::shared_ptr<STI::Engine::SequenceResult> bufferedResult;
    if (sequenceBuffer.get(id.seqID, bufferedResult)) {
        bufferedResult->addShotResult(id.seqIndex, shotID, shotStatus);
    }
    return false;
}

bool LocalPersistenceManager::saveSequence(const std::shared_ptr<STI::Engine::SequenceResult>& sequenceResult, bool isOwner)
{
    if (sequenceResult == 0) return false;

    std::shared_ptr<PersistenceManager> delegate;
    bool hasDelegate = false;

    if (hasDelegate && isOwner && delegate != 0) {
        if(delegate->saveSequence(sequenceResult, true)) {     //transfer ownership to delegate
            return true;
        }
        //if failed, save locally
    }
    
    if (!sequenceBuffer.contains(sequenceResult->seqid) ) {
        addToBuffer(sequenceResult);
    }

    // sequenceBuffer.get()
    //would help to have a way to update a sequence;  updateSequence(sequenceID, shotResult)


    if (isOwner) {
        return saveSequenceLocal(sequenceResult, isOwner);
    }

    return true;
}

bool LocalPersistenceManager::saveSequenceLocal(const std::shared_ptr<STI::Engine::SequenceResult>& sequenceResult, bool isOwner)
{
    if (sequenceResult == 0) return false;

    std::shared_ptr<STI::Engine::ShotRepository> repo;
    if (!getShotRepository(repo)) return false;

    // //this should be called when the shot is saved
    // if (repo->findSequenceResult(sequenceResult->seqid)) {
    //     repo->updateSequence();
    // }

    ResultsPaths resultsPaths = repo->preparePaths(sequenceResult->seqid);    //e.g., make directory sturcture

    bool success = repo->saveSequence(sequenceResult->seqid, sequenceResult);

    if (!success) {
        success = defaultRepository->saveSequence(sequenceResult->seqid, sequenceResult);
    }

    return success;
}


bool isPartial(const std::shared_ptr<ShotResult>& shotResult)
{
    return false;
}


bool LocalPersistenceManager::getResultsPaths(const STI::Engine::ShotID& sid, ResultsPaths& resultsPaths)
{
    std::shared_ptr<STI::Engine::ShotRepository> repo;
    if (!getShotRepository(repo)) return false;

    if (sid.parseID.shotConfig.shotType == ShotType::SingleUndocumented) {
        repo = transientRepository;
    }

    // std::shared_ptr<ShotResult> finalShotResult = shotResult;

    //happens on (remote) delegate, where data should be saved.
    resultsPaths = repo->preparePaths(sid);    //e.g., make directory sturcture

    return true;
}

//replace resultsDocumenter with localDocumenter and make it a function argument.
//allow engine to call saveShotLocal with a TransientResultsDocumenter which auto deletes itself
//after going out of scope. Data is copied from this documenter into a MixedValue when read() is called.
bool LocalPersistenceManager::saveShotLocal(const STI::Engine::ShotID& sid, 
                                            const std::shared_ptr<FullShotResult>& fullShotResult, bool isOwner)
{
    if (fullShotResult == 0) return false;

    //bool isShotOwner = sid.parseID.jobSourceID == localDeviceID;

    std::shared_ptr<STI::Engine::ShotRepository> repo;
    if (!getShotRepository(repo)) return false;

    if (sid.parseID.shotConfig.shotType == ShotType::SingleUndocumented) {
        repo = transientRepository;
    }

    // std::shared_ptr<ShotResult> finalShotResult = shotResult;

    //happens on (remote) delegate, where data should be saved.
    ResultsPaths resultsPaths = repo->preparePaths(sid);    //e.g., make directory sturcture
    // auto collector = std::make_shared<LocalResultsCollector>(sid, resultsPaths, fileHolderFactory);
    auto collector = resultsCollectorFactory->createResultsCollector(sid, resultsPaths, fileHolderFactory);

    auto shotRecord = transferResults(collector, fullShotResult->shotResult, isOwner);   //collector is passed on to all devices in shot
    collector->setRecord(shotRecord);

    transferParseResult(fullShotResult->parseResult, resultsPaths.timingPath);

    auto completeResult = std::make_shared<FullShotResult>();  
    completeResult->parseResult = fullShotResult->parseResult;
    completeResult->shotResult = collector->getResults();

    // addToBuffer(completeResult);     //causes infinite recursion with call to saveShotLocal

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

    std::shared_ptr<STI::Utils::FileServer> remoteFileServer;

    auto stackTraceData = parseResult->stackTraceResult->stackTraceData;
    auto& files = stackTraceData->getTimingFiles();

    auto commonBase = STI::Utils::FileID::commonBasePath(files);    //deepest common path of files

    for (auto& fileID : files) {

        fs::path localPath = timingPath;
        fs::path filePath = fileID.path;
        localPath /= filePath.lexically_relative(commonBase);   //relative directory of this file
        localPath /= fileID.filename;

        auto uniqueFilename = STI::Utils::makeUniquePath( localPath.string() );
        fs::path uniquePath = uniqueFilename;

        auto localFileHandle = fileHolderFactory->makeFileHolder(uniquePath.parent_path(), uniquePath.filename());

        if (stackTraceData->getFileServer(remoteFileServer)) {
            //transfer file to local
            remoteFileServer->transferFile(fileID, localFileHandle, STI::Utils::FileTransferType::Binary);
            stackTraceData->replaceFile(fileID.getFullFilename(), localFileHandle->getID());       
        }

        // file->transferFile(localFileHandle);    //transfer file to local
        // stackTraceData->replaceFile(inputFilename, localFileHandle);            

    }
}

void LocalPersistenceManager::handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& mess)
{
    using STI::Device::EngineSchedulerMessage;

    if (mess == 0) return;

    if (mess->originalSourceID() != localDeviceID) return;


    switch(mess->schedulerMessageType) {
        case EngineSchedulerMessage::SchedulerMessageType::ParseComplete:
            //change shot status in sequenceResult
            break;
        case EngineSchedulerMessage::SchedulerMessageType::PlayComplete:
        
            break;
    }

}

void LocalPersistenceManager::addSequence(const std::shared_ptr<STI::Engine::SequenceResult>& sequenceResult)
{
    std::shared_ptr<PersistenceManager> delegate;
    bool hasDelegate = false;

    if (hasDelegate && delegate != 0) {
        delegate->addSequence(sequenceResult);  //transfer ownership to delegate
        return;
    }
    //if failed, save locally
    
    if (saveSequenceLocal(sequenceResult, true)) {
        addToBuffer(sequenceResult);
    }
}


bool LocalPersistenceManager::getSequenceResult(const STI::Engine::SequenceID& id, std::shared_ptr<STI::Engine::SequenceResult>& sequenceResult)
{
    if (sequenceBuffer.get(id, sequenceResult) && (sequenceResult != 0) ) {
        return true;
    }
    //not found in buffer; check repository
    return false;
}

bool LocalPersistenceManager::getSequenceLocal(const STI::Engine::SequenceID& seqid, std::shared_ptr<STI::Engine::SequenceResult>& sequenceResult)
{
    bool success = false;
    std::shared_ptr<STI::Engine::ShotRepository> repo;

    if (getShotRepository(repo)) {
        success = repo->getSequenceResult(seqid, sequenceResult);
    }

    if (!success) {
        success = defaultRepository->getSequenceResult(seqid, sequenceResult);
    }

    return success;
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

