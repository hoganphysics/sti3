#include "TransientRepository.h"

#include <sti/engine/FullShotResult.h>
#include <sti/engine/ParseID.h>
#include <sti/engine/ParseResult.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/SequenceResult.h>
#include <sti/engine/ShotResult.h>
#include <sti/utils/utils.h>

#include <filesystem>

using STI::Engine::TransientRepository;
using STI::Engine::ResultsPaths;
using STI::Engine::ShotID;
using STI::Engine::ShotResult;
using STI::Engine::MeasurementVector;
using STI::Engine::MeasurementMap;
using STI::Engine::ParseID;
using STI::Engine::ParseResult;
using STI::Engine::FullShotResult;
using STI::Engine::SequenceID;
using STI::Engine::SequenceEntryID;
using STI::Engine::EngineJobStatus;
using STI::Engine::SequenceResult;



TransientRepository::TransientRepository(const std::string& tempBasePath)
: parseBuffer(5), resultBuffer(5), sequenceBuffer(5)
{
    //Need a temporary directory to store any files (measurements, timing files, etc).
    //This directory is for temporary files that will be deleted when the shot expires.

    std::filesystem::path uniqueBasePath = tempBasePath;
    uniqueBasePath /= "transient_cache";
    uniqueBasePath /= "tmp";

    //The directory will be cleared regularly, so we need to make sure to use a unique path.
    //tempResultsPath = STI::Utils::makeUniquePath( uniqueBasePath.string() );    // tmp_0, tmp_1, etc.
    tempResultsPath = uniqueBasePath.string();  //Fix: using fixed path for now to avoid many tmp directories
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

bool TransientRepository::findParseResult(const ParseID& pid)
{
    return parseBuffer.contains(pid);
}

bool TransientRepository::findShotResult(const ShotID& sid)
{
    return resultBuffer.contains(sid);
}

bool TransientRepository::findSequenceResult(const SequenceID& seqid)
{
    return sequenceBuffer.contains(seqid);
}

bool TransientRepository::getParseResult(const ParseID& id, std::shared_ptr<ParseResult>& parseResult)
{
    return parseBuffer.get(id, parseResult) && (parseResult != 0);
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

bool TransientRepository::getSequenceResult(const SequenceID& id, std::shared_ptr<SequenceResult>& sequenceResult)
{
    return sequenceBuffer.get(id, sequenceResult) && (sequenceResult != 0);
}

bool TransientRepository::saveShot(const ShotID& sid, const std::shared_ptr<FullShotResult>& fullShotResult)
{
    if (fullShotResult == 0) return false;

    std::shared_ptr<FullShotResult> expiredResult;  //the oldest result in the buffer; ready to delete
    if (resultBuffer.addAndRemove(sid, fullShotResult, expiredResult) && expiredResult != 0) {
        //The buffer was full. Need to delete the old result;
        ShotResult::deleteFiles(*expiredResult->shotResult);
    }

    std::shared_ptr<ParseResult> expiredParseResult;  //the oldest result in the buffer; ready to delete
    if (parseBuffer.addAndRemove(sid.parseID, fullShotResult->parseResult, expiredParseResult) && expiredParseResult != 0) {
        //The buffer was full. Need to delete the old result;
        ParseResult::deleteFiles(*expiredParseResult);
    }

    return findShotResult(sid);
}

ResultsPaths TransientRepository::preparePaths(const ShotID& sid)
{
    return preparePaths();
}

ResultsPaths TransientRepository::preparePaths(const SequenceID& seqid)
{
    return preparePaths();
}

ResultsPaths TransientRepository::preparePaths()
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


bool TransientRepository::TransientRepository::getMeasurements(const ShotID& sid, std::shared_ptr<MeasurementMap>& measurements)
{
    std::shared_ptr<FullShotResult> fullShotResult;

    if (resultBuffer.get(sid, fullShotResult) && fullShotResult != 0 
        && fullShotResult->shotResult != 0 && fullShotResult->shotResult->measurements != 0) {
        measurements = fullShotResult->shotResult->measurements;
        return (measurements != 0);
    }

    return false;
}

bool TransientRepository::updateSequence(const SequenceEntryID& id, const ShotID& shotID, const EngineJobStatus& shotStatus)
{
    std::shared_ptr<SequenceResult> sequenceResult;
    bool success = false;
    
    if (getSequenceResult(id.seqID, sequenceResult)) {
        success = sequenceResult->addShotResult(id.seqIndex, shotID, shotStatus);
    }
    return success;
}

bool TransientRepository::saveSequence(const SequenceID& seqid, const std::shared_ptr<SequenceResult>& sequenceResult)
{
    if (sequenceResult == 0) return false;

    std::shared_ptr<SequenceResult> expiredResult;  //the oldest result in the buffer; ready to delete
    if (sequenceBuffer.addAndRemove(seqid, sequenceResult, expiredResult) && expiredResult != 0) {
        //expiredResult deleted
    }
    return findSequenceResult(seqid);
}

