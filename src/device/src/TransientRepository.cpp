#include "TransientRepository.h"

#include <sti/engine/FullShotResult.h>
#include <sti/engine/ParseID.h>
#include <sti/engine/ParseResult.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/SequenceResult.h>
#include <sti/engine/ShotResult.h>
#include <sti/utils/utils.h>

#include <filesystem>
#include <system_error>

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

namespace {

bool ensureDirectoryExists(const std::string& pathName)
{
    std::filesystem::path path = pathName;
    if (path.empty()) {
        return false;
    }

    std::error_code ec;
    bool exists = std::filesystem::exists(path, ec);
    if (ec) {
        return false;
    }

    if (!exists) {
        std::filesystem::create_directories(path, ec);
        if (ec) {
            return false;
        }
    }

    return std::filesystem::is_directory(path, ec) && !ec;
}

} // namespace



TransientRepository::TransientRepository(const std::string& tempBasePath, const std::shared_ptr<STI::Utils::FileServer>& fileServer)
: parseBuffer(5), resultBuffer(5), sequenceBuffer(5), fileServer(fileServer)
{
    //Need a temporary directory to store any files (measurements, timing files, etc).
    //This directory is for temporary files that will be deleted when the shot expires.

    std::filesystem::path uniqueBasePath = tempBasePath;
    uniqueBasePath /= "transient_cache";
    uniqueBasePath /= "tmp";

    // Keep one stable path and create it lazily when requested or when a FileHolder opens a file.
    tempResultsPath = uniqueBasePath.string();
}

TransientRepository::~TransientRepository()
{
    std::filesystem::path repoPath = tempResultsPath;

    //Delete the temporary directory and all contents
    std::error_code ec;
    std::filesystem::remove_all(repoPath, ec);
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
        if (expiredResult->shotResult != 0) {
            ShotResult::deleteFiles(*expiredResult->shotResult, fileServer);
        }
    }

    std::shared_ptr<ParseResult> expiredParseResult;  //the oldest result in the buffer; ready to delete
    if (parseBuffer.addAndRemove(sid.parseID, fullShotResult->parseResult, expiredParseResult) && expiredParseResult != 0) {
        //The buffer was full. Need to delete the old result;
        ParseResult::deleteFiles(*expiredParseResult, fileServer);
    }

    return findShotResult(sid);
}

std::string TransientRepository::prepareLogPath(const STI::Utils::TimeStamp& timeStamp, bool autocreate)
{
    auto paths = preparePaths();
    if (autocreate) {
        ensureDirectoryExists(paths.basePath);
    }
    return paths.basePath;
}

ResultsPaths TransientRepository::preparePaths(const ShotID& sid)
{
    return preparePaths();
}

ResultsPaths TransientRepository::preparePaths(const ParseID& pid)
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
    paths.parsePath = tempResultsPath;
    paths.experimentPath = tempResultsPath;
    paths.sequencePath = tempResultsPath;
    paths.tempPath = tempResultsPath;
    paths.timingPath = tempResultsPath;

    return paths;
}

std::string TransientRepository::getTemporaryPath() const
{
    if (!ensureDirectoryExists(tempResultsPath)) {
        return "";
    }

    return tempResultsPath;
}


bool TransientRepository::getMeasurements(const ShotID& sid, std::shared_ptr<MeasurementMap>& measurements)
{
    std::shared_ptr<FullShotResult> fullShotResult;

    if (resultBuffer.get(sid, fullShotResult) && fullShotResult != 0 
        && fullShotResult->shotResult != 0 && fullShotResult->shotResult->measurements != 0) {
        measurements = fullShotResult->shotResult->measurements;
        return (measurements != 0);
    }

    return false;
}

bool TransientRepository::saveSequenceParseResult(const SequenceEntryID& id, const std::shared_ptr<ParseResult>& parseResult, const EngineJobStatus& parseStatus)
{
    if (parseResult == 0) return false;

    std::shared_ptr<ParseResult> expiredParseResult;
    if (parseBuffer.addAndRemove(parseResult->pid, parseResult, expiredParseResult) && expiredParseResult != 0) {
        ParseResult::deleteFiles(*expiredParseResult, fileServer);
    }

    std::shared_ptr<SequenceResult> sequenceResult;
    if (sequenceBuffer.get(id.seqID, sequenceResult) && sequenceResult != 0) {
        sequenceResult->status[id.seqIndex] = parseStatus;
    }

    return findParseResult(parseResult->pid);
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
