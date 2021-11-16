
#include "TransientRepository.h"
#include "ShotResult.h"
#include "RawEvent.h"
#include "utils.h"

#include <filesystem>

using STI::Engine::TransientRepository;
using STI::Engine::ResultsPaths;
using STI::Engine::ShotID;
using STI::Engine::ShotResult;
using STI::Engine::MeasurementVector;


TransientRepository::TransientRepository(const std::string& tempBasePath)
: resultBuffer(5)
{
    //Need a temporary directory to store any files (measurements, timing files, etc).
    //This directory is for temporary files that will be deleted when the shot expires.

    std::filesystem::path uniqueBasePath = tempBasePath;
    uniqueBasePath /= "transient_cache";
    uniqueBasePath /= "tmp";

    //The directory will be cleared regularly, so we need to make sure to use a unique path.
    tempResultsPath = STI::Utils::makeUniquePath( uniqueBasePath.string() );    // tmp_0, tmp_1, etc.
    std::filesystem::path repoPath = tempResultsPath;

    if (!std::filesystem::exists(repoPath)) {
        std::filesystem::create_directories(repoPath);
    }
}

TransientRepository::~TransientRepository()
{
    std::filesystem::path repoPath = tempResultsPath;
    
    //Delete the temporary directory and all contents
    if (std::filesystem::exists(repoPath)) {
        std::filesystem::remove_all(repoPath);
    }
}

bool TransientRepository::findShot(const ShotID& sid)
{
    return resultBuffer.contains(sid);
}

bool TransientRepository::getShot(const ShotID& id, std::shared_ptr<ShotResult>& shotResult)
{
    return resultBuffer.get(id, shotResult);
}

bool TransientRepository::saveShot(const ShotID& sid, const std::shared_ptr<ShotResult>& shotResult)
{
    std::shared_ptr<ShotResult> expiredResult;  //the oldest result in the buffer; ready to delete
    
    if (resultBuffer.addAndRemove(sid, shotResult, expiredResult) && expiredResult != 0) {
        //The buffer was full. Need to delete the old result;
        ShotResult::deleteShotFiles(*expiredResult);
    }

    return findShot(sid);
}

ResultsPaths TransientRepository::preparePaths(const ShotID& sid)
{
    ResultsPaths paths;

    paths.basePath = tempResultsPath;
    paths.dataPath = tempResultsPath;
    paths.experimentPath = tempResultsPath;
    paths.sequencePath = tempResultsPath;
    paths.tempPath = tempResultsPath;
    paths.timingPath = tempResultsPath;

    return paths;
}

bool TransientRepository::TransientRepository::getMeasurements(const ShotID& sid, std::shared_ptr<MeasurementVector>& measurements)
{
    std::shared_ptr<ShotResult> shotResult;

    if (resultBuffer.get(sid, shotResult) && shotResult != 0 && shotResult->measurements != 0) {
        measurements = shotResult->measurements;
        return true;
    }

    return false;
}

