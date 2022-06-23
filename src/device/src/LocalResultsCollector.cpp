
#include "LocalResultsCollector.h"
#include <sti/engine/ShotID.h>
//#include "EventEngine.h"
#include "ParsedDependencyTree.h"
#include <sti/engine/Measurement.h>
#include <sti/engine/RawEvent.h>
#include "ShotResult.h"
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


LocalResultsCollector::LocalResultsCollector(const STI::Engine::ShotID& sid, 
                //    const std::shared_ptr<STI::Engine::EventEngine>& eventEngine, 
                //    const std::shared_ptr<ParsedDependencyTree>& dependencies,
                    const ResultsPaths& paths,
                    const std::shared_ptr<STI::Utils::FileHolderFactory>& factory)
: resultsPaths(paths), fileHolderFactory(factory)
{
    shotResult = std::make_shared<ShotResult>();
    shotResult->sid = sid;
}


ShotID LocalResultsCollector::getShotID() const
{
    return shotResult->sid;
}

// std::shared_ptr<ParsedDependencyTree> LocalResultsCollector::getDependencies()
// {
//     return dependencies;
// }


void LocalResultsCollector::setRecord(const ShotResultRecord& shotRecord)
{
    std::unique_lock<std::mutex> collectorLock(collectorMutex);

    if (shotResult != 0) {
        shotResult->shotResultRecord = shotRecord;
    }
}

std::shared_ptr<ShotResult> LocalResultsCollector::getResults() const
{
    return shotResult;
}

void LocalResultsCollector::addEvents(const DeviceEventMap& parsedEvents)
{
    std::unique_lock<std::mutex> collectorLock(collectorMutex);

    if (shotResult == 0) return;
    
    // resultsTicket.events = parsedEvents;
    shotResult->parsedEvents = std::move(parsedEvents);
}

void LocalResultsCollector::addTimingFiles(const std::vector<std::shared_ptr<STI::Utils::FileHolder>>& files)
{
    std::unique_lock<std::mutex> collectorLock(collectorMutex);

    if (shotResult == 0) return;

    for (auto& file : files) {
        std::string localPath = makeLocalPath(resultsPaths.timingPath, file->getFilename());
        auto localFileHandle = fileHolderFactory->makeFileHolder(localPath);

        if (file->transferFile(localFileHandle)) {
            shotResult->timingFiles.push_back(localFileHandle);
        }
    }
}


bool LocalResultsCollector::addMeasurements(const std::shared_ptr<MeasurementVector>& measurements)
{
    std::unique_lock<std::mutex> collectorLock(collectorMutex);

    if (shotResult == 0 || measurements == 0) return false;

    if (shotResult->measurements == 0 || shotResult->measurements->size() == 0) {
        shotResult->measurements = measurements;
    }
    else {
        //vector contains shared_ptr so deep copy is inexpensive
        shotResult->measurements->insert(shotResult->measurements->end(), measurements->begin(), measurements->end());
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
    return shotResult->measurements;
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

    if (shotResult == 0) return false;

    (shotResult->attributes)[deviceID] = attributes;
    return true;
}