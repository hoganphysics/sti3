#include <catch2/catch_test_macros.hpp>

#include <sti/engine/EngineJobSourceID.h>
#include <sti/engine/EngineJobStatus.h>
#include <sti/engine/ParseID.h>
#include <sti/engine/ParseResult.h>
#include <sti/engine/ParsedVar.h>
#include <sti/engine/Sequence.h>
#include <sti/engine/SequenceID.h>
#include <sti/engine/SequenceResult.h>
#include <sti/engine/ShotID.h>

#include "LegacyShotRepository.h"

#include <tinyxml2.h>

#include <atomic>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <system_error>

using STI::Engine::EngineJobSourceID;
using STI::Engine::EngineJobStatus;
using STI::Engine::LegacyShotRepository;
using STI::Engine::ParseID;
using STI::Engine::ParseResult;
using STI::Engine::Sequence;
using STI::Engine::SequenceEntryID;
using STI::Engine::SequenceID;
using STI::Engine::SequenceIndex;
using STI::Engine::SequenceResult;
using STI::Engine::SequenceType;
using STI::Engine::ShotID;

namespace {

class ScopedRepositoryPath
{
public:
    explicit ScopedRepositoryPath(const std::string& testName)
    {
        static std::atomic<unsigned> counter{0};
        path = std::filesystem::temp_directory_path()
            / ("sti3_legacy_repository_" + testName + "_" + std::to_string(counter++));
    }

    ~ScopedRepositoryPath()
    {
        std::error_code error;
        std::filesystem::remove_all(path, error);
    }

    std::filesystem::path path;
};

std::shared_ptr<SequenceResult> makeSequenceResult(const EngineJobSourceID& source, unsigned entryCount = 1)
{
    auto sequence = std::make_shared<Sequence>(SequenceType::Closed);
    for (unsigned index = 0; index < entryCount; ++index) {
        sequence->addEntry(SequenceIndex(index, 0), {});
    }
    return std::make_shared<SequenceResult>(SequenceID::generateUniqueID(source), sequence);
}

ShotID makeShotID(const EngineJobSourceID& source, const SequenceEntryID& entryID)
{
    auto parseID = ParseID::generateUniqueID(source, entryID);
    return ShotID::generateUniqueID(parseID, source);
}

std::filesystem::path sequenceFilename(const std::filesystem::path& sequencePath, const SequenceID& id)
{
    return sequencePath / (
        "seq_" + id.timestamp.date_YYYY_MM_DD("-") + "_" + id.timestamp.time_hh_mm_ss_mmmuuunnn() + ".xml");
}

std::string readFile(const std::filesystem::path& filename)
{
    std::ifstream file(filename);
    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

tinyxml2::XMLElement* findExperiment(tinyxml2::XMLElement* experiments, int index)
{
    if (experiments == nullptr) return nullptr;

    for (auto experiment = experiments->FirstChildElement("experiment");
         experiment != nullptr;
         experiment = experiment->NextSiblingElement("experiment")) {
        auto sequenceIndex = experiment->FirstChildElement("sequenceindex");
        auto indexElement = sequenceIndex == nullptr ? nullptr : sequenceIndex->FirstChildElement("index");
        if (indexElement != nullptr && indexElement->IntText(-1) == index) {
            return experiment;
        }
    }

    return nullptr;
}

unsigned experimentCount(tinyxml2::XMLElement* experiments)
{
    unsigned count = 0;
    if (experiments == nullptr) return count;

    for (auto experiment = experiments->FirstChildElement("experiment");
         experiment != nullptr;
         experiment = experiment->NextSiblingElement("experiment")) {
        ++count;
    }
    return count;
}

} // namespace

TEST_CASE("LegacyShotRepository sequence entries link parse and experiment XML",
          "[legacyshotrepository][sequence]")
{
    ScopedRepositoryPath repositoryPath("sequence_entry_files");
    LegacyShotRepository repository(repositoryPath.path.string());
    EngineJobSourceID source("legacy-repository-test", "localhost");

    auto sequenceResult = makeSequenceResult(source, 2);
    auto sequenceID = sequenceResult->seqid;
    SequenceEntryID completedEntry(sequenceID, SequenceIndex(0, 0));
    SequenceEntryID canceledEntry(sequenceID, SequenceIndex(1, 0));

    REQUIRE(repository.saveSequence(sequenceID, sequenceResult));

    auto completedShot = makeShotID(source, completedEntry);
    REQUIRE(repository.updateSequence(completedEntry, completedShot, EngineJobStatus::Completed));

    auto canceledParse = std::make_shared<ParseResult>();
    canceledParse->pid = ParseID::generateUniqueID(source, canceledEntry);
    REQUIRE(repository.saveSequenceParseResult(canceledEntry, canceledParse, EngineJobStatus::Canceled));

    auto sequencePath = sequenceFilename(repository.preparePaths(sequenceID).sequencePath, sequenceID);
    tinyxml2::XMLDocument document;
    REQUIRE(document.LoadFile(sequencePath.string().c_str()) == tinyxml2::XML_SUCCESS);

    auto series = document.FirstChildElement("series");
    REQUIRE(series != nullptr);

    auto sequence = series->FirstChildElement("sequence");
    REQUIRE(sequence != nullptr);
    REQUIRE(sequence->FirstChildElement("current") != nullptr);
    CHECK(sequence->FirstChildElement("current")->UnsignedText() == 2);
    REQUIRE(sequence->FirstChildElement("expected") != nullptr);
    CHECK(sequence->FirstChildElement("expected")->UnsignedText() == 2);

    auto experiments = series->FirstChildElement("experiments");
    REQUIRE(experiments != nullptr);
    CHECK(experimentCount(experiments) == 2);

    auto completedExperiment = findExperiment(experiments, 0);
    REQUIRE(completedExperiment != nullptr);
    REQUIRE(completedExperiment->FirstChildElement("status") != nullptr);
    CHECK(std::string(completedExperiment->FirstChildElement("status")->GetText()) == "Completed");
    REQUIRE(completedExperiment->FirstChildElement("parse") != nullptr);
    CHECK(completedExperiment->FirstChildElement("parse")->FirstChildElement("file") != nullptr);
    CHECK(completedExperiment->FirstChildElement("file") != nullptr);

    auto canceledExperiment = findExperiment(experiments, 1);
    REQUIRE(canceledExperiment != nullptr);
    REQUIRE(canceledExperiment->FirstChildElement("status") != nullptr);
    CHECK(std::string(canceledExperiment->FirstChildElement("status")->GetText()) == "Canceled");
    REQUIRE(canceledExperiment->FirstChildElement("parse") != nullptr);
    CHECK(canceledExperiment->FirstChildElement("parse")->FirstChildElement("file") != nullptr);
    CHECK(canceledExperiment->FirstChildElement("file") == nullptr);
}

TEST_CASE("LegacyShotRepository sequence saves are idempotent across cache eviction",
          "[legacyshotrepository][sequence]")
{
    ScopedRepositoryPath repositoryPath("idempotent_sequence");
    LegacyShotRepository repository(repositoryPath.path.string());
    EngineJobSourceID source("legacy-repository-test", "localhost");

    auto firstResult = makeSequenceResult(source);
    auto firstID = firstResult->seqid;
    SequenceEntryID firstEntry(firstID, SequenceIndex(0, 0));

    REQUIRE(repository.saveSequence(firstID, firstResult));
    REQUIRE(repository.updateSequence(
        firstEntry, makeShotID(source, firstEntry), EngineJobStatus::Completed));

    auto firstPath = sequenceFilename(repository.preparePaths(firstID).sequencePath, firstID);
    REQUIRE(std::filesystem::exists(firstPath));
    REQUIRE(readFile(firstPath).find("<current>1</current>") != std::string::npos);

    // A repeated save while the builder is cached is a successful no-op.
    auto oneShotContents = readFile(firstPath);
    REQUIRE(repository.saveSequence(firstID, firstResult));
    CHECK(readFile(firstPath) == oneShotContents);

    // The first builder must remain live while five sequences are active.
    for (int i = 0; i < 4; ++i) {
        auto result = makeSequenceResult(source);
        REQUIRE(repository.saveSequence(result->seqid, result));
    }
    REQUIRE(repository.updateSequence(
        firstEntry, makeShotID(source, firstEntry), EngineJobStatus::Completed));
    REQUIRE(readFile(firstPath).find("<current>1</current>") != std::string::npos);

    tinyxml2::XMLDocument document;
    REQUIRE(document.LoadFile(firstPath.string().c_str()) == tinyxml2::XML_SUCCESS);
    auto series = document.FirstChildElement("series");
    REQUIRE(series != nullptr);
    CHECK(experimentCount(series->FirstChildElement("experiments")) == 1);

    // Adding a sixth sequence evicts the first builder. Re-saving the first
    // sequence must preserve its completed-shot XML rather than rebuilding it.
    auto sixthResult = makeSequenceResult(source);
    REQUIRE(repository.saveSequence(sixthResult->seqid, sixthResult));

    auto completedContents = readFile(firstPath);
    REQUIRE(repository.saveSequence(firstID, firstResult));
    CHECK(readFile(firstPath) == completedContents);
    CHECK(readFile(firstPath).find("<current>1</current>") != std::string::npos);
}
