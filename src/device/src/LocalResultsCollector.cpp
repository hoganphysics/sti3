
#include "LocalResultsCollector.h"
#include <sti/engine/ShotID.h>
//#include "EventEngine.h"
#include "ParsedDependencyTree.h"
#include <sti/engine/Measurement.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/ShotResult.h>
#include <sti/engine/ParseResult.h>
// #include "EngineParseResult.h"

#include <sti/engine/FullShotResult.h>

#include <sti/utils/utils.h>
#include <sti/engine/ShotResultRecord.h>


#include <filesystem>
namespace fs = std::filesystem;

using STI::Engine::LocalResultsCollector;
using STI::Engine::ShotID;
using STI::Engine::ParsedDependencyTree;
using STI::Engine::Measurement;
using STI::Engine::MeasurementVector;
using STI::Engine::ShotResult;
using STI::Engine::ShotResultRecord;
using STI::Engine::ParseResult;



LocalResultsCollector::LocalResultsCollector(const STI::Engine::ShotID& sid, 
                //    const std::shared_ptr<STI::Engine::EventEngine>& eventEngine, 
                //    const std::shared_ptr<ParsedDependencyTree>& dependencies,
                    const ResultsPaths& paths,
                    const std::shared_ptr<STI::Utils::FileHolderFactory>& factory)
: resultsPaths(paths), fileHolderFactory(factory)
{
    fullShotResult = std::make_shared<FullShotResult>();
    fullShotResult->shotResult = std::make_shared<ShotResult>();
    fullShotResult->shotResult->sid = sid;

    // parseResult = std::make_shared<ParseResult>();
    // parseResult->pid = sid.parseID;
}


ShotID LocalResultsCollector::getShotID() const
{
    return fullShotResult->shotResult->sid;
}

// std::shared_ptr<ParsedDependencyTree> LocalResultsCollector::getDependencies()
// {
//     return dependencies;
// }


void LocalResultsCollector::setRecord(const ShotResultRecord& shotRecord)
{
    std::unique_lock<std::mutex> collectorLock(collectorMutex);

    if (fullShotResult != 0 && fullShotResult->shotResult != 0) {
        fullShotResult->shotResult->shotResultRecord = shotRecord;
    }
}

std::shared_ptr<ShotResult> LocalResultsCollector::getResults() const
{
    return fullShotResult->shotResult;
}

std::shared_ptr<ParseResult> LocalResultsCollector::getParseResults() const
{
    return fullShotResult->parseResult;
}

// void LocalResultsCollector::addEvents(const DeviceEventMap& parsedEvents)
// {
//     std::unique_lock<std::mutex> collectorLock(collectorMutex);

//     if (fullShotResult == 0 || fullShotResult->shotResult == 0) return;
    
//     // resultsTicket.events = parsedEvents;
//     fullShotResult->parseResult->baseEventGroup = parsedEvents; //need to convert to a group
//     // shotResult->
//     fullShotResult->shotResult->engineParseResult.parsedEvents = std::move(parsedEvents);
// }

// void LocalResultsCollector::addTimingFiles(const std::vector<std::shared_ptr<STI::Utils::FileHolder>>& files)
// {
//     std::unique_lock<std::mutex> collectorLock(collectorMutex);

//     if (fullShotResult->shotResult == 0) return;

//     for (auto& file : files) {
//         std::string localPath = makeLocalPath(resultsPaths.timingPath, file->getFilename());
//         auto localFileHandle = fileHolderFactory->makeFileHolder(localPath);

//         if (file->transferFile(localFileHandle)) {
//             fullShotResult->parseResult->timingFiles.push_back(localFileHandle);
//         }
//     }
// }


bool LocalResultsCollector::addMeasurements(const std::shared_ptr<MeasurementVector>& measurements)
{
    std::unique_lock<std::mutex> collectorLock(collectorMutex);

    if (fullShotResult == 0 || fullShotResult->shotResult == 0 || measurements == 0) return false;

    if (fullShotResult->shotResult->measurements == 0 || fullShotResult->shotResult->measurements->size() == 0) {
        fullShotResult->shotResult->measurements = measurements;
    }
    else {
        //vector contains shared_ptr so deep copy is inexpensive
        fullShotResult->shotResult->measurements->insert(fullShotResult->shotResult->measurements->end(), measurements->begin(), measurements->end());
    }
    

    bool success = true;

    //on results destination
    STI::Utils::MixedValue filedata;

    for (auto& meas : *measurements) {
        if (meas != 0 && meas->data().getType() == STI::Utils::MixedValueType::File) {
            //possibly do this in 
            meas->extractMeasurementResult(filedata);
            auto fileHandle = filedata.getFile();

            // std::string localPath = resultsDocumenter->makeLocalPath(sid, fileHandle->getFilename());
            
            std::string localPath = makeLocalPath(resultsPaths.dataPath, fileHandle->getFilename());
            
            auto localFileHandle = fileHolderFactory->makeFileHolder(localPath);
            
            //transfer file to local
            if (fileHandle->transferFile(localFileHandle)) {
                //sucess; delete remote?
                success &= true;
                filedata.setValue(localFileHandle);
                meas->setMeasurementResult(filedata);
            }
            else {
                success = false;
            }
        }

    }
    return success;
}

std::shared_ptr<MeasurementVector> LocalResultsCollector::getMeasurements()
{
    return fullShotResult->shotResult->measurements;
}

std::string LocalResultsCollector::makeLocalPath(const std::string& basePath, const std::string& remoteFilename)
{
    fs::path remotePath = remoteFilename;
    fs::path localPath = basePath;

    localPath /= remotePath.filename();

    return STI::Utils::makeUniquePath( localPath.string() );
}



bool LocalResultsCollector::addAttributes(const STI::Device::DeviceID& deviceID, const std::map<std::string, std::string>& attributes)
{
    std::unique_lock<std::mutex> collectorLock(collectorMutex);

    if (fullShotResult == 0 || fullShotResult->shotResult == 0) return false;

    (fullShotResult->shotResult->attributes)[deviceID] = attributes;
    return true;
}