#include "LocalResultsCollector.h"

#include <sti/engine/FullShotResult.h>
#include <sti/engine/Measurement.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/ParseResult.h>
#include <sti/engine/ShotID.h>
#include <sti/engine/ShotResult.h>
#include <sti/engine/ShotResultRecord.h>
#include <sti/utils/utils.h>

#include "ParsedDependencyTree.h"

#include <filesystem>
namespace fs = std::filesystem;

using STI::Engine::LocalResultsCollector;
using STI::Engine::ShotID;
using STI::Engine::ParsedDependencyTree;
using STI::Engine::Measurement;
using STI::Engine::MeasurementVector;
using STI::Engine::MeasurementMap;
using STI::Engine::ShotResult;
using STI::Engine::ShotResultRecord;
using STI::Engine::ParseResult;


LocalResultsCollector::LocalResultsCollector(const STI::Engine::ShotID& sid, 
                                             const ResultsPaths& paths,
                                             const std::shared_ptr<STI::Utils::FileHolderFactory>& factory)
: resultsPaths(paths), fileHolderFactory(factory)
{
    fullShotResult = std::make_shared<FullShotResult>();
    fullShotResult->shotResult = std::make_shared<ShotResult>();
    fullShotResult->shotResult->sid = sid;
}


ShotID LocalResultsCollector::getShotID() const
{
    if (fullShotResult != 0 && fullShotResult->shotResult != 0) {
        return fullShotResult->shotResult->sid;
    }
    ShotID sid;
    return sid;
}


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

bool LocalResultsCollector::addMeasurements(const STI::Device::DeviceID& deviceID, const MeasurementVector& measurements)
{
    std::unique_lock<std::mutex> collectorLock(collectorMutex);

    if (fullShotResult == 0 || fullShotResult->shotResult == 0) return false;

    if (measurements.size() == 0) return true;

    if (fullShotResult->shotResult->measurements == 0) {
        fullShotResult->shotResult->measurements = std::make_shared<STI::Engine::MeasurementMap>();
    }

    auto& devMeasurements = (*fullShotResult->shotResult->measurements)[deviceID];

    //vector contains shared_ptr so deep copy is inexpensive
    devMeasurements.insert(devMeasurements.end(), measurements.begin(), measurements.end());

    bool success = true;

    //on results destination
    STI::Utils::MixedValue filedata;

    for (auto& meas : measurements) {
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

std::shared_ptr<MeasurementMap> LocalResultsCollector::getMeasurements()
{
    if (fullShotResult != 0 && fullShotResult->shotResult != 0 && fullShotResult->shotResult->measurements != 0) {
        return fullShotResult->shotResult->measurements;
    }

    auto empty = std::make_shared<MeasurementMap>();
    return empty;
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

