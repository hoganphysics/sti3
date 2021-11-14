
#include "LocalPersistenceManager.h"
#include "LocalResultsCollector.h"
#include "DeviceID.h"
#include "EventEngineJob.h"
#include "EventEngine.h"
#include "EventEngineDependencyTree.h"
#include "ResultsDocumenter.h"
#include "RawEvent.h"

#include <filesystem>

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


LocalPersistenceManager::LocalPersistenceManager(const std::shared_ptr<STI::Utils::FileHolderFactory>& fileHolderFactory,
        const std::shared_ptr<STI::Engine::ResultsDocumenter>& resultsDocumenter,
        const std::shared_ptr<STI::Engine::ShotRepository>& shotRepository)
: fileHolderFactory(fileHolderFactory), resultsDocumenter(resultsDocumenter), shotRepository(shotRepository)
{
    //documenter = std::make_shared<DefaultResultsDocumenter>();    //save to local .sti dir
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

void LocalPersistenceManager::setResultsDocumenter(const std::shared_ptr<STI::Engine::ResultsDocumenter>& documenter)
{
    resultsDocumenter = documenter;
}

void LocalPersistenceManager::setShotRepository(const std::shared_ptr<STI::Engine::ShotRepository>& repo)
{
    shotRepository = repo;
}

bool LocalPersistenceManager::getShotRepository(std::shared_ptr<STI::Engine::ShotRepository>& repo)
{
    repo = shotRepository;
    return (repo != 0);
}

bool LocalPersistenceManager::transferResults(const std::shared_ptr<STI::Engine::ResultsCollector>& resultsCollector)
{
    if (resultsCollector == 0 || shotRepository == 0) return false;

    auto sid = resultsCollector->getShotID();

    if (shotRepository->findShot(sid) ) {
        std::shared_ptr<STI::Engine::MeasurementVector> measurements;
        
        if (shotRepository->getMeasurements(sid, measurements)) {
            //resultsCollector->addAttributes();
            return resultsCollector->addMeasurements(measurements);
        }
    }

    return false;
}

bool LocalPersistenceManager::saveShot(const STI::Engine::ShotID& sid, const std::shared_ptr<STI::Engine::EventEngine>& eventEngine)
{
    std::shared_ptr<ResultsCollector> collector;
    std::shared_ptr<PersistenceManager> delegate;

    if (sid.parseID.shotType == STI::Engine::ParseID::ShotType::SingleUndocumented) {
        //Skip documentation
        return true;
    }

    std::set<unsigned> priorities;
    delegatePriorities.getKeys(priorities);

    std::vector<std::shared_ptr<PersistenceManager>> orderedDelegates;

    for (auto& p : priorities) {
        DeviceID id;
        delegatePriorities.get(p, id);

        if (delegates.get(id, delegate) && delegate != 0) {
            orderedDelegates.push_back(delegate);
        }
    }

    if (orderedDelegates.size() > 0) {
        return orderedDelegates.front()->saveShot(sid, eventEngine);
        //other delegates?
    }
    else {
        return saveShotLocal(sid, eventEngine);
    }

    return false;
}


//replace resultsDocumenter with localDocumenter and make it a function argument.
//allow engine to call saveShotLocal with a TransientResultsDocumenter which auto deletes itself
//after going out of scope. Data is copied from this documenter into a MixedValue when read() is called.
bool LocalPersistenceManager::saveShotLocal(const STI::Engine::ShotID& sid, const std::shared_ptr<STI::Engine::EventEngine>& eventEngine)
{
    if (resultsDocumenter == 0) return false;

    //happens on (remote) delegate, where data should be served.
    std::shared_ptr<LocalResultsCollector> collector;
    
    ResultsPaths resultsPaths = resultsDocumenter->preparePaths(sid);    //e.g., make directory sturcture

    collector = std::make_shared<LocalResultsCollector>(sid, eventEngine->getParsedTree(), 
                                                        resultsPaths, fileHolderFactory);

    eventEngine->transferResults(collector);   //collector is passed on to all devices in shot

    return resultsDocumenter->save(resultsPaths, collector);
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


bool LocalPersistenceManager::getResultTicket(const ShotID& sid, std::shared_ptr<ResultTicket>& ticket)
{
    std::shared_ptr<ShotRepository> remoteRepo; 
    std::shared_ptr<PersistenceManager> delegate;

    std::set<unsigned> priorities;
    delegatePriorities.getKeys(priorities);

    // std::vector<std::shared_ptr<PersistenceManager>> orderedDelegates;

    bool found = false;

    for (auto& p : priorities) {
        DeviceID id;
        delegatePriorities.get(p, id);

        if (delegates.get(id, delegate) && delegate != 0 && delegate->getShotRepository(remoteRepo)) {
            if (remoteRepo->findShot(sid)) {
                ticket = std::make_shared<ResultTicket>(sid, remoteRepo, ResultTicket::TicketStatus::Complete);
                found = true;
                break;
            }
        }
    }

    if (!found && shotRepository != 0 && shotRepository->findShot(sid)) {
        ticket = std::make_shared<ResultTicket>(sid, shotRepository, ResultTicket::TicketStatus::Complete);
        found = true;
    }

    //use LocalShotRepository
    //ticket = std::make_shared<>(shotRepository);

    return found && (ticket != 0);
}


// std::shared_ptr<STI::Engine::ResultsCollector> LocalPersistenceManager::createResultsCollector(const STI::Engine::ShotID& sid, const std::shared_ptr<STI::Engine::EventEngine>& eventEngine)
// {
//     std::shared_ptr<ResultsCollector> collector;
//     // collector = std::make_shared<LocalResultsCollector>(job.getJobID().sid);

//     std::shared_ptr<PersistenceManager> delegate;

//     std::set<unsigned> priorities;
//     delegatePriorities.getKeys(priorities);

//     std::vector<std::shared_ptr<STI::Engine::ResultsCollector>> collectors;

//     for (auto& p : priorities) {
//         DeviceID id;
//         delegatePriorities.get(p, id);

//         if (delegates.get(id, delegate) && delegate != 0) {
            
//             collector = delegate->createResultsCollector(sid, eventEngine);

//             collectors.push_back(collector);
//         }
//     }

//     if (collectors.size() == 0) {
//         collector = std::shared_ptr<LocalResultsCollector>(sid, eventEngine, resultsDocumenter);
// //        collector = std::shared_ptr<DefaultLocalResultsCollector>(this);    //saves to .sti
//         collectors.push_back(collector);  
//     }

//     return collectors.front();
// }

// void LocalPersistenceManager::saveShot(const std::shared_ptr<STI::Engine::ResultsCollector>& collector)
// {

//     //get top delegate
//     //  delegate->saveShot
//     //in no top delegate
//     //  saveShotLocally

//     // if(collector->collectMeasurements()) {
//     //     collector->save();
//     // }

//     std::shared_ptr<PersistenceManager> delegate;

//     std::set<unsigned> priorities;
//     delegatePriorities.getKeys(priorities);


//     if (priorities.size() > 0) {
//         DeviceID id;
//         delegatePriorities.get(*(priorities.begin()), id);

//         if (delegates.get(id, delegate) && delegate != 0) {
//             delegate->saveShot(collector);
//         }
//     }
//     else {
//         //save locally
//         auto engine = collector->getEngine();

//         engine->transferMeasurements(collector);

//         collector->save();

//     }

// }

