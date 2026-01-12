#include "LegacyShotRepository.h"

#include "LegacyExperimentXMLBuilder.h"
#include "LegacyParseXMLBuilder.h"
#include "LegacySequenceXMLBuilder.h"

#include <sti/engine/ParseResult.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <iostream>

#include "CerealArchives.h"
#include <cereal/types/map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

#include <tinyxml2.h>


using STI::Engine::LegacyShotRepository;
using STI::Engine::ResultsPaths;
using STI::Engine::ShotID;
using STI::Engine::SequenceID;
using STI::Engine::SequenceResult;
using STI::Utils::TimeStamp;


LegacyShotRepository::LegacyShotRepository(const std::string& baseDevicePath)
: cachedPaths(5), cachedSequences(3), baseDevicePath(baseDevicePath)
{
}

LegacyShotRepository::~LegacyShotRepository()
{   
}

std::string LegacyShotRepository::prepareLogPath(const STI::Utils::TimeStamp& timeStamp, bool autocreate)
{
    auto logPath = getLogBasePath(timeStamp);

    if(autocreate) {
        makePathIfNew(logPath);
    }

    return logPath;
}


std::string LegacyShotRepository::getLogBasePath(const TimeStamp& timeStamp)
{
    std::filesystem::path basePath(baseDevicePath);
    basePath /= "logs";
    basePath /= timeStamp.date_YYYY_MM_DD("/");

    return basePath.string();
}


ResultsPaths LegacyShotRepository::preparePaths(const ShotID& sid)
{
    return preparePaths(sid.submissionTime);
}

ResultsPaths LegacyShotRepository::preparePaths(const SequenceID& seqid)
{
    ResultsPaths paths = makePaths(seqid.timestamp);

    makePathIfNew(paths.sequencePath);

    return paths;
}


bool LegacyShotRepository::findParseResult(const ParseID& pid)
{
    auto paths = makePaths(pid.parseTimestamp);

    std::filesystem::path parsePath = paths.experimentPath;
    parsePath /= makeParseFilename(pid);

    return std::filesystem::exists(parsePath);
}

bool LegacyShotRepository::findShotResult(const ShotID& sid)
{
    auto paths = makePaths(sid.submissionTime);

    std::filesystem::path shotPath = paths.experimentPath;
    shotPath /= makeShotFilename(sid);

    return std::filesystem::exists(shotPath);
}

bool LegacyShotRepository::findSequenceResult(const SequenceID& seqid)
{
    auto paths = makePaths(seqid.timestamp);

    std::filesystem::path seqPath = paths.sequencePath;
    seqPath /= makeSequenceFilename(seqid);

    return std::filesystem::exists(seqPath);
}


bool LegacyShotRepository::getParseResult(const ParseID& id, std::shared_ptr<ParseResult>& parseResult)
{
    if (!findParseResult(id)) return false;

    auto paths = makePaths(id.parseTimestamp);

    std::filesystem::path serializePath = paths.experimentPath;
    serializePath /= makeParseFilename(id);
    {
        std::ifstream file(serializePath.string());
        if (!file.good() || file.peek() == std::ifstream::traits_type::eof()) {
            // file exists but is empty
            return false;
        }

        try {
            cereal::XMLInputArchive archive(file);

            auto loadedResult = std::make_shared<STI::Engine::ParseResult>();
            archive(loadedResult);
            parseResult = loadedResult;
        }
        catch (const cereal::Exception& e) {
            //failed to load - likely due to version mismatch or corruption
            parseResult.reset();
            return false;
        }
    }

    return true;
}

bool LegacyShotRepository::getShotResult(const ShotID& id, std::shared_ptr<ShotResult>& shotResult)
{
    if (!findShotResult(id)) return false;

    auto paths = makePaths(id.submissionTime);

    // std::filesystem::path serializePath = paths.dataPath;
    // serializePath /= archiveFilename;
    std::filesystem::path serializePath = paths.experimentPath;
    serializePath /= makeShotFilename(id);
    {
        // std::ifstream file( serializePath.string() );
        // cereal::XMLInputArchive archive( file );  
        
        // shotResult = std::make_shared<STI::Engine::ShotResult>();

        // archive(shotResult);
    }

    return (shotResult != 0);
}

bool LegacyShotRepository::getSequenceResult(const SequenceID& id, std::shared_ptr<SequenceResult>& sequenceResult)
{
    if (!findSequenceResult(id)) return false;

    auto paths = makePaths(id.timestamp);

    std::filesystem::path serializePath = paths.sequencePath;
    serializePath /= makeSequenceFilename(id);

    {
        // std::ifstream file( serializePath.string() );
        // cereal::XMLInputArchive archive( file );  
        
        // sequenceResult = std::make_shared<STI::Engine::SequenceResult>();

        // archive(sequenceResult);
    }

    return (sequenceResult != 0);
}


bool LegacyShotRepository::getMeasurements(const ShotID& sid, std::shared_ptr<MeasurementMap>& measurements)
{
    return false;
}


bool LegacyShotRepository::saveShot(const ShotID& sid, const std::shared_ptr<FullShotResult>& fullShotResult)
{
    if (fullShotResult == 0) return false;

    auto parsePaths = preparePaths(sid.parseID.parseTimestamp);
    auto paths = preparePaths(sid);

    std::filesystem::path targetParsePath = parsePaths.experimentPath;
    std::filesystem::path targetShotPath = paths.experimentPath;

    targetParsePath /= makeParseFilename(sid.parseID);
    targetShotPath /= makeShotFilename(sid);

    if (!std::filesystem::exists(targetParsePath)) {
        LegacyParseXMLBuilder parseBuilder(targetParsePath.string(), fullShotResult->parseResult);
        parseBuilder.build();
        parseBuilder.write();
    }

    LegacyExperimentXMLBuilder builder(targetShotPath.string(), fullShotResult);

    if (sid.parseID.shotType == ShotType::SequenceEntry) {
        std::shared_ptr<LegacySequenceXMLBuilder> seqBuilder;
        if (cachedSequences.get(sid.parseID.sequenceEntryID.seqID, seqBuilder) && seqBuilder != 0) {            
            builder.addSequenceFilename(seqBuilder->getFilename());
        }
    }
    builder.build();
    builder.write();

    return true;
}


bool LegacyShotRepository::updateSequence(const SequenceEntryID& id, const ShotID& shotID, const EngineJobStatus& shotStatus)
{
    std::shared_ptr<LegacySequenceXMLBuilder> builder;

    if (cachedSequences.get(id.seqID, builder) && builder != 0) {
        auto paths = preparePaths(shotID);

        std::filesystem::path targetShotPath = paths.experimentPath;
        targetShotPath /= makeShotFilename(shotID);

        builder->addShot(targetShotPath.string(), id);
        builder->write();
    }

    return true;
}

bool LegacyShotRepository::saveSequence(const SequenceID& seqID, const std::shared_ptr<SequenceResult>& sequenceResult)
{
    if (sequenceResult == 0) return false;

    if (cachedSequences.contains(seqID)) {
        return false;
    }

    auto seqPaths = preparePaths(seqID);

    std::filesystem::path targetSeqPath = seqPaths.sequencePath;

    targetSeqPath /= makeSequenceFilename(seqID);

    auto builder = std::make_shared<LegacySequenceXMLBuilder>(targetSeqPath.string(), sequenceResult);
    cachedSequences.add(seqID, builder);
    builder->write();

    // LegacySequenceXMLBuilder builder();
    return true;
}

std::string LegacyShotRepository::makeParseFilename(const ParseID& pid)
{
    return "parse_" + pid.parseTimestamp.time_hh_mm_ss_mmmuuunnn() + ".xml";
}

std::string LegacyShotRepository::makeShotFilename(const ShotID& sid)
{
    return "shot_" + sid.submissionTime.time_hh_mm_ss_mmmuuunnn() + ".xml";
}

std::string LegacyShotRepository::makeSequenceFilename(const SequenceID& seqid)
{
    return "seq_" + seqid.timestamp.time_hh_mm_ss_mmmuuunnn() + ".xml";
}

ResultsPaths LegacyShotRepository::makePaths(const TimeStamp& timeStamp)
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

    auto experimentPath = uniqueBasePath / "experiments";
    paths.experimentPath = experimentPath.string();

    auto sequencePath = uniqueBasePath / "sequences";
    paths.sequencePath = sequencePath.string();

    // auto logPath = uniqueBasePath / "logs";
    // paths.logPath = logPath.string();

    cachedPaths.add(timeStamp, paths);

    return paths;
}

ResultsPaths LegacyShotRepository::preparePaths(const TimeStamp& timeStamp)
{
    ResultsPaths paths = makePaths(timeStamp);

    makePathIfNew(paths.basePath);
    makePathIfNew(paths.tempPath);
    makePathIfNew(paths.dataPath);
    makePathIfNew(paths.timingPath);
    makePathIfNew(paths.experimentPath);

    return paths;
}

void LegacyShotRepository::makePathIfNew(const std::string& pathName)
{
    std::filesystem::path newPath = pathName;
    if (!std::filesystem::exists(newPath)) {
        std::filesystem::create_directories(newPath);
    }
}

std::string LegacyShotRepository::getShotBasePath(const TimeStamp& timeStamp)
{
    std::filesystem::path basePath(baseDevicePath);
    basePath /= "shot_cache";
    basePath /= timeStamp.date_YYYY_MM_DD();

    return basePath.string();
}
