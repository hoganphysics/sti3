
#include "TransientRepository.h"
#include "ShotResult.h"
#include <sti/engine/RawEvent.h>
#include <sti/utils/utils.h>
#include <sti/engine/ParseID.h>
#include "ParseResult.h"
#include "FullShotResult.h"

#include <filesystem>

using STI::Engine::TransientRepository;
using STI::Engine::ResultsPaths;
using STI::Engine::ShotID;
using STI::Engine::ShotResult;
using STI::Engine::MeasurementVector;
using STI::Engine::ParseID;
using STI::Engine::ParseResult;
using STI::Engine::FullShotResult;



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

bool TransientRepository::findShotResult(const ShotID& sid)
{
    return resultBuffer.contains(sid);
}

bool TransientRepository::findParseResult(const ParseID& sid)
{
    return false;
}

bool TransientRepository::getShotResult(const ShotID& id, std::shared_ptr<ShotResult>& shotResult)
{
    std::shared_ptr<FullShotResult> fullShotResult;
    if (resultBuffer.get(id, fullShotResult) && fullShotResult != 0) {
        shotResult = fullShotResult->shotResult;
        return (shotResult != 0);
    }
    return false;
}

bool TransientRepository::getParseResult(const ParseID& id, std::shared_ptr<ParseResult>& shotResult)
{
    return false;
}

bool TransientRepository::saveShot(const ShotID& sid, const std::shared_ptr<FullShotResult>& fullShotResult)
{
    std::shared_ptr<FullShotResult> expiredResult;  //the oldest result in the buffer; ready to delete
    
    if (resultBuffer.addAndRemove(sid, fullShotResult, expiredResult) && expiredResult != 0) {
        //The buffer was full. Need to delete the old result;
        ShotResult::deleteShotFiles(*expiredResult->shotResult);
    }

    return findShotResult(sid);
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
    std::shared_ptr<FullShotResult> fullShotResult;

    if (resultBuffer.get(sid, fullShotResult) && fullShotResult != 0 
        && fullShotResult->shotResult != 0 && fullShotResult->shotResult->measurements != 0) {
        measurements = fullShotResult->shotResult->measurements;
        return true;
    }

    return false;
}

