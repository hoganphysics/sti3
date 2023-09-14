#include "LocalResultsCollector.h"
#include "DefaultImageWriter.h"

#include <sti/engine/FullShotResult.h>
#include <sti/engine/Measurement.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/ParsedDependencyTree.h>
#include <sti/engine/ParseResult.h>
#include <sti/engine/ShotID.h>
#include <sti/engine/ShotResult.h>
#include <sti/engine/ShotResultRecord.h>

#include <sti/utils/Image.h>
#include <sti/utils/utils.h>

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
using STI::Utils::FileServer;
using STI::Device::DeviceID;


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

bool LocalResultsCollector::addMeasurements(const DeviceID& deviceID, const MeasurementVector& measurements, const std::shared_ptr<FileServer>& sourceFileServer)
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
            auto remoteFileID = filedata.getFileID();     //remote file, to be transfered

            

            // std::string localPath = resultsDocumenter->makeLocalPath(sid, fileHandle->getFilename());
            
            auto localFileHandle = makeLocalFileHandle(resultsPaths.dataPath, remoteFileID);
            
            // auto localFileHandle = fileHolderFactory->makeFileHolder(localPath);
            
            //transfer file to local
            if (sourceFileServer != 0 && 
                sourceFileServer->transferFile(remoteFileID, localFileHandle, STI::Utils::FileTransferType::Binary)) 
            {
                //sucess; delete remote?
                success &= true;
                filedata.setValue(localFileHandle->getID());
                meas->setMeasurementResult(filedata);
            }
            else {
                success = false;
            }

        }
        else if (meas != 0 && meas->data().getType() == STI::Utils::MixedValueType::Image) {
            STI::Utils::MixedValue imagedata;
            meas->extractMeasurementResult(imagedata);
            
            auto imageWritter = std::make_shared<STI::Utils::DefaultImageWriter>(fileHolderFactory);

            auto imageHandle = filedata.getImage();
            imageHandle->writeToFile(imageWritter, resultsPaths.dataPath);

            std::shared_ptr<STI::Utils::FileHolder> fileHandle;
            
            // //transfer file to local
            // if (imageHandle->getFile(fileHandle) && fileHandle->transferFile(localFileHandle)) {
            //     //sucess; delete remote?
            //     success &= true;
            //     filedata.setValue(localFileHandle);
            //     meas->setMeasurementResult(filedata);
            // }
            // else {
            //     success = false;
            // }
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

// std::string LocalResultsCollector::makeLocalPath(const std::string& basePath, const std::string& remoteFilename)
std::shared_ptr<STI::Utils::FileHolder> LocalResultsCollector::makeLocalFileHandle(const std::string& basePath, const STI::Utils::FileID& remoteFileID)
{
    fs::path remotePath = remoteFileID.filename;
    fs::path localPath = basePath;

    localPath /= remotePath.filename();

    auto uniqueLocalFilename = STI::Utils::makeUniquePath( localPath.string() );
    fs::path uniqueLocalPath = uniqueLocalFilename;

    auto localFileHandle = fileHolderFactory->makeFileHolder(uniqueLocalPath.parent_path(), uniqueLocalPath.filename());

    return localFileHandle;
}


bool LocalResultsCollector::addAttributes(const STI::Device::DeviceID& deviceID, const std::map<std::string, std::string>& attributes)
{
    std::unique_lock<std::mutex> collectorLock(collectorMutex);

    if (fullShotResult == 0 || fullShotResult->shotResult == 0) return false;

    (fullShotResult->shotResult->attributes)[deviceID] = attributes;
    return true;
}

