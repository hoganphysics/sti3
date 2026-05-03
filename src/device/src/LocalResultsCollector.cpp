#include "LocalResultsCollector.h"
//#include "DefaultImageWriter.h"

#include <sti/engine/EnginePlayingMessage.h>
#include <sti/engine/FullShotResult.h>
#include <sti/engine/Measurement.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/ParsedDependencyTree.h>
#include <sti/engine/ParseResult.h>
#include <sti/engine/ShotID.h>
#include <sti/engine/ShotResult.h>
#include <sti/engine/ShotResultRecord.h>

#include <sti/utils/BinaryData.h>
#include <sti/utils/Image.h>
#include <sti/utils/utils.h>

#include <filesystem>
#include <limits>
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
using STI::Engine::EnginePlayingMessage;


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

    //Need to cache FileIDs during transfer to make sure to only transfer each file
    //and image/binary data once, since Measurements can share references.
    std::map<STI::Utils::FileID, STI::Utils::FileID> cachedIDs; 
    std::map<const void*, STI::Utils::FileID> cachedBinaryDataIDs;

    for (auto& meas : measurements) {

        if (meas == 0) continue;

        STI::Utils::MixedValue data;
        meas->extractMeasurementResult(data);
        success &= transferValue(data, cachedIDs, cachedBinaryDataIDs, sourceFileServer);
        meas->setMeasurementResult(std::move(data));
    }
    return success;
}

bool LocalResultsCollector::transferValue(STI::Utils::MixedValue& data,
                                          std::map<STI::Utils::FileID, STI::Utils::FileID>& cachedFileIDs,
                                          std::map<const void*, STI::Utils::FileID>& cachedBinaryDataIDs,
                                          const std::shared_ptr<STI::Utils::FileServer>& sourceFileServer)
{
    bool success = false;

    if (data.getType() == STI::Utils::MixedValueType::Vector) {
        success = true;
        for (auto& v : data.vec()) {
            success &= transferValue(v, cachedFileIDs, cachedBinaryDataIDs, sourceFileServer);
        }
    }
    else if (data.getType() == STI::Utils::MixedValueType::File) {
        
        auto remoteFileID = data.getFileID();     //remote file, to be transfered

        auto it = cachedFileIDs.find(remoteFileID);

        if (it == cachedFileIDs.end()) {
            //new file
            auto localFileHandle = makeLocalFileHandle(resultsPaths.dataPath, remoteFileID);

            //transfer file to local
            if (sourceFileServer != 0 &&
                sourceFileServer->transferFile(remoteFileID, localFileHandle, STI::Utils::FileTransferType::Binary))
            {
                success = true;
                sourceFileServer->deleteFile(remoteFileID);     //delete remote

                cachedFileIDs[remoteFileID] = localFileHandle->getID();
                data.setValue(cachedFileIDs[remoteFileID]);
            }
            
        }
        else {
            //already transferred this file
            success = true;
            data.setValue(it->second);
        }

    }
    else if (data.getType() == STI::Utils::MixedValueType::Image) {

        auto imageHandle = data.getImage();
        auto remoteFileID = imageHandle->getFileID();   //images have a unique FileID, even if stored as BinaryData

        auto it = cachedFileIDs.find(remoteFileID);

        if (it == cachedFileIDs.end()) {
            //new image
            auto localFileHandle = makeLocalFileHandle(resultsPaths.dataPath, remoteFileID);

            if (sourceFileServer != 0 && imageHandle->write(sourceFileServer, localFileHandle)) {
                //success
                success = true;
                cachedFileIDs[remoteFileID] = localFileHandle->getID();
                imageHandle->setImageData(localFileHandle);
            }
        }
        else {
            //already transferred this image
            success = true;
            data.setValue(it->second);
        }        
    }
    else if (data.getType() == STI::Utils::MixedValueType::Binary) {

        auto binaryData = data.getBinary();

        if (binaryData != 0) {
            auto it = cachedBinaryDataIDs.find(binaryData.get());

            if (it != cachedBinaryDataIDs.end()) {
                success = true;
                data.setValue(it->second);
            }
            else {
                auto localFileHandle = makeLocalBinaryDataFileHandle(resultsPaths.dataPath);

                if (localFileHandle != 0 &&
                    binaryData->bytes() <= std::numeric_limits<unsigned>::max() &&
                    localFileHandle->openFile())
                {
                    success = true;

                    if (binaryData->bytes() > 0) {
                        char* bytes = nullptr;
                        success = binaryData->getBytes(bytes) &&
                                  bytes != nullptr &&
                                  localFileHandle->write(bytes, static_cast<unsigned>(binaryData->bytes()));
                    }

                    localFileHandle->closeFile();

                    if (success) {
                        cachedBinaryDataIDs[binaryData.get()] = localFileHandle->getID();
                        data.setValue(localFileHandle->getID());
                    }
                }
            }
        }
    }
    else {
        //do nothing for all other MixedValue types
        success = true;
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

std::shared_ptr<STI::Utils::FileHolder> LocalResultsCollector::makeLocalFileHandle(const std::string& basePath, const STI::Utils::FileID& remoteFileID)
{
    fs::path remotePath = remoteFileID.filename;
    fs::path localPath = basePath;

    localPath /= remotePath.filename();

    auto uniqueLocalFilename = STI::Utils::makeUniquePath( localPath.string());
    fs::path uniqueLocalPath = uniqueLocalFilename;
    uniqueLocalPath.make_preferred();

    auto localFileHandle = fileHolderFactory->makeFileHolder(uniqueLocalPath.parent_path().string(), uniqueLocalPath.filename().string());

    return localFileHandle;
}

std::shared_ptr<STI::Utils::FileHolder> LocalResultsCollector::makeLocalBinaryDataFileHandle(const std::string& basePath)
{
    fs::path localPath = basePath;
    localPath /= "binary_measurement.bin";

    auto uniqueLocalFilename = STI::Utils::makeUniquePath(localPath.string());
    fs::path uniqueLocalPath = uniqueLocalFilename;
    uniqueLocalPath.make_preferred();

    auto localFileHandle = fileHolderFactory->makeFileHolder(uniqueLocalPath.parent_path().string(), uniqueLocalPath.filename().string());

    return localFileHandle;
}


bool LocalResultsCollector::addAttributes(const STI::Device::DeviceID& deviceID, const std::map<std::string, std::string>& attributes)
{
    std::unique_lock<std::mutex> collectorLock(collectorMutex);

    if (fullShotResult == 0 || fullShotResult->shotResult == 0) return false;

    (fullShotResult->shotResult->attributes)[deviceID] = attributes;
    return true;
}

bool LocalResultsCollector::addVersionInfo(const STI::Device::DeviceID& deviceID, const std::vector<STI::Device::VersionInfo>& versions)
{
    std::unique_lock<std::mutex> collectorLock(collectorMutex);

    if (fullShotResult == 0 || fullShotResult->shotResult == 0) return false;

    (fullShotResult->shotResult->versions)[deviceID] = versions;
    return true;
}

bool LocalResultsCollector::addMessages(const std::vector<EnginePlayingMessage>& messages)
{
    std::unique_lock<std::mutex> collectorLock(collectorMutex);

    if (fullShotResult == 0 || fullShotResult->shotResult == 0) return false;

    fullShotResult->shotResult->messages.insert(fullShotResult->shotResult->messages.end(), messages.begin(), messages.end());
    return true;
}
