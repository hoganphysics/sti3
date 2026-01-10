#include "SerializedRepository.h"

#include <sti/engine/FullShotResult.h>
#include <sti/engine/Measurement.h>
#include <sti/engine/ParseResult.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/ShotID.h>
#include <sti/engine/ShotResult.h>
#include <sti/engine/SequenceID.h>
#include <sti/engine/SequenceResult.h>

#include "LocalResultsCollector.h"

#include <filesystem>
#include <fstream>

#include "CerealArchives.h"
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>

using STI::Engine::SerializedRepository;
using STI::Engine::ResultsPaths;
using STI::Engine::ShotID;
using STI::Engine::LocalResultsCollector;
using STI::Engine::SequenceID;
using STI::Engine::SequenceResult;
using STI::Utils::TimeStamp;


SerializedRepository::SerializedRepository(const std::string& baseDevicePath)
: cachedPaths(5), baseDevicePath(baseDevicePath)
{
}

std::string SerializedRepository::makeParseFilename(const ParseID& pid)
{
    return "parse_" + pid.parseTimestamp.time_hh_mm_ss_mmmuuunnn() + ".xml";
}

std::string SerializedRepository::makeShotFilename(const ShotID& sid)
{
    return "shot_" + sid.submissionTime.time_hh_mm_ss_mmmuuunnn() + ".xml";
}

std::string SerializedRepository::makeSequenceFilename(const SequenceID& seqid)
{
    return "seq_" + seqid.timestamp.time_hh_mm_ss_mmmuuunnn() + ".xml";
}


bool SerializedRepository::findParseResult(const ParseID& pid)
{
    auto paths = makePaths(pid.parseTimestamp);

    std::filesystem::path parsePath = paths.experimentPath;
    parsePath /= makeParseFilename(pid);

    return std::filesystem::exists(parsePath);
}

bool SerializedRepository::findShotResult(const ShotID& sid)
{
    auto paths = makePaths(sid.submissionTime);

    std::filesystem::path shotPath = paths.experimentPath;
    shotPath /= makeShotFilename(sid);

    return std::filesystem::exists(shotPath);
}

bool SerializedRepository::findSequenceResult(const SequenceID& seqid)
{
    auto paths = makePaths(seqid.timestamp);

    std::filesystem::path seqPath = paths.sequencePath;
    seqPath /= makeSequenceFilename(seqid);

    return std::filesystem::exists(seqPath);
}

bool SerializedRepository::getMeasurements(const ShotID& sid, std::shared_ptr<MeasurementMap>& measurements)
{
    std::shared_ptr<ShotResult> shotResult;
    
    if (getShotResult(sid, shotResult) && shotResult != 0 && shotResult->measurements != 0) {
        measurements = shotResult->measurements;
        return (measurements != 0);
    }

    return false;
}

ResultsPaths SerializedRepository::preparePaths(const ShotID& sid)
{
    return preparePaths(sid.submissionTime);
}

ResultsPaths SerializedRepository::preparePaths(const SequenceID& seqid)
{
    ResultsPaths paths = makePaths(seqid.timestamp);

    makePathIfNew(paths.sequencePath);

    return paths;
}

std::string SerializedRepository::prepareLogPath(const STI::Utils::TimeStamp& timeStamp, bool autocreate)
{
    auto logPath = getLogBasePath(timeStamp);

    if(autocreate) {
        makePathIfNew(logPath);
    }

    return logPath;
}


ResultsPaths SerializedRepository::preparePaths(const TimeStamp& timeStamp)
{
    ResultsPaths paths = makePaths(timeStamp);

    makePathIfNew(paths.basePath);
    makePathIfNew(paths.tempPath);
    makePathIfNew(paths.dataPath);
    makePathIfNew(paths.timingPath);
    makePathIfNew(paths.experimentPath);

    return paths;
}

void SerializedRepository::makePathIfNew(const std::string& pathName)
{
    std::filesystem::path newPath = pathName;
    if (!std::filesystem::exists(newPath)) {
        std::filesystem::create_directories(newPath);
    }
}


bool SerializedRepository::saveShot(const STI::Engine::ShotID& sid, const std::shared_ptr<FullShotResult>& fullShotResult)
{
    if (fullShotResult == 0) return false;

    auto parsePaths = preparePaths(sid.parseID.parseTimestamp);
    auto paths = preparePaths(sid);

    std::filesystem::path serializeParsePath = parsePaths.experimentPath;
    std::filesystem::path serializeShotPath = paths.experimentPath;

    serializeParsePath /= makeParseFilename(sid.parseID);
    serializeShotPath /= makeShotFilename(sid);

    if (!std::filesystem::exists(serializeParsePath)) {
        //only save ParseResult if not already saved

        std::ofstream file( serializeParsePath.string() );
        cereal::XMLOutputArchive archive( file );

        archive( fullShotResult->parseResult );
    }

    {
        std::ofstream file( serializeShotPath.string() );
        cereal::XMLOutputArchive archive( file );

        archive( fullShotResult->shotResult );
    }

    return true;
}

bool SerializedRepository::updateSequence(const SequenceEntryID& id, const ShotID& shotID, const EngineJobStatus& shotStatus)
{
    if (!findSequenceResult(id.seqID)) return false;

    // auto paths = preparePaths(id.seqID);

    //write to streaming xml

    return true;
}

bool SerializedRepository::saveSequence(const SequenceID& seqid, const std::shared_ptr<SequenceResult>& sequenceResult)
{
    if (sequenceResult == 0) return false;

    auto paths = preparePaths(seqid);

    std::filesystem::path serializePath = paths.sequencePath;
    serializePath /= makeSequenceFilename(seqid);
    {
        std::ofstream file( serializePath.string() );
        cereal::XMLOutputArchive archive( file );

        archive( sequenceResult );
    }

    return true;
}


bool SerializedRepository::getParseResult(const ParseID& id, std::shared_ptr<ParseResult>& parseResult)
{
    if (!findParseResult(id)) return false;

    auto paths = preparePaths(id.parseTimestamp);

    std::filesystem::path serializePath = paths.experimentPath;
    serializePath /= makeParseFilename(id);
    {
        std::ifstream file( serializePath.string() );
        if (!file.good() || file.peek() == std::ifstream::traits_type::eof()) {
            // file exists but is empty
            return false;
        }

        try {
            cereal::XMLInputArchive archive( file );  
        
            auto loadedResult = std::make_shared<STI::Engine::ParseResult>();
            archive(loadedResult);
            parseResult = loadedResult;
        }
        catch (const cereal::Exception &e) {
            //failed to load - likely due to version mismatch or corruption
            parseResult.reset();
            return false;
        }
    }

    return true;
}


bool SerializedRepository::getShotResult(const ShotID& sid, std::shared_ptr<ShotResult>& shotResult)
{
    if (!findShotResult(sid)) return false;

    auto paths = preparePaths(sid);

    std::filesystem::path serializePath = paths.experimentPath;
    serializePath /= makeShotFilename(sid);
    {
        std::ifstream file( serializePath.string() );
        if (!file.good() || file.peek() == std::ifstream::traits_type::eof()) {
            // file exists but is empty
            return false;
        }

        try {
            cereal::XMLInputArchive archive( file );  
        
            auto loadedResult = std::make_shared<STI::Engine::ShotResult>();
            archive(loadedResult);
            shotResult = loadedResult;
        }
        catch (const cereal::Exception &e) {
            //failed to load - likely due to version mismatch or corruption
            shotResult.reset();
            return false;
        }
    }

    return true;
}


bool SerializedRepository::getSequenceResult(const SequenceID& id, std::shared_ptr<SequenceResult>& sequenceResult)
{
    if (!findSequenceResult(id)) return false;

    auto paths = preparePaths(id);

    std::filesystem::path serializePath = paths.sequencePath;
    serializePath /= makeSequenceFilename(id);
    {
        std::ifstream file( serializePath.string() );
        if (!file.good() || file.peek() == std::ifstream::traits_type::eof()) {
            // file exists but is empty
            return false;
        }

        try {
            cereal::XMLInputArchive archive( file );  
        
            auto seq = std::make_shared<STI::Engine::Sequence>();
            auto loadedResult = std::make_shared<STI::Engine::SequenceResult>(id, seq);
            archive(loadedResult);
            sequenceResult = loadedResult;
        }
        catch (const cereal::Exception &e) {
            //failed to load - likely due to version mismatch or corruption
            sequenceResult.reset();
            return false;
        }
    }

    return true;
}

ResultsPaths SerializedRepository::makePaths(const TimeStamp& timeStamp)
{
    std::unique_lock<std::mutex> pathLock(pathMutex);

    ResultsPaths paths;

    if (cachedPaths.get(timeStamp, paths)) {
        return paths;
    }

    std::filesystem::path uniqueBasePath = getShotBasePath(timeStamp);

    paths.basePath = uniqueBasePath.string();

    auto time = timeStamp.time_hh_mm_ss_mmmuuunnn();       //shots stored by timestamp

    auto tempPath = uniqueBasePath / "temp" / time;
    paths.tempPath = tempPath.string();

    auto dataPath = uniqueBasePath / "data" / time;
    paths.dataPath = dataPath.string();

    auto timingPath = uniqueBasePath / "timing" / time;
    paths.timingPath = timingPath.string();

    auto experimentPath = uniqueBasePath / "experiments" / time;
    paths.experimentPath = experimentPath.string();

    auto sequencePath = uniqueBasePath / "sequences" / time;
    paths.sequencePath = sequencePath.string();

    cachedPaths.add(timeStamp, paths);

    return paths;
}


std::string SerializedRepository::getShotBasePath(const TimeStamp& timeStamp)
{

    std::filesystem::path basePath(baseDevicePath);
    basePath /= "shot_cache";
    basePath /= timeStamp.date_YYYY_MM_DD();
    // basePath /= sid.submissionTime.time_hh_mm_ss_mmmuuunnn();       //shots stored by timestamp
    //auto uniqueBasePath = basePath / sid.playTime.time_hh_mm_ss_mmmuuunnn();

    return basePath.string();
}

std::string SerializedRepository::getLogBasePath(const TimeStamp& timeStamp)
{
    std::filesystem::path basePath(baseDevicePath);
    basePath /= "logs";
    basePath /= timeStamp.date_YYYY_MM_DD("/");

    return basePath.string();
}
