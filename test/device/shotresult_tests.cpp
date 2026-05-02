#include <catch2/catch_test_macros.hpp>

#include <sti/engine/EngineJobSourceID.h>
#include <sti/engine/FullShotResult.h>
#include <sti/engine/ParseID.h>
#include <sti/engine/ParseResult.h>
#include <sti/engine/ShotID.h>
#include <sti/engine/ShotResult.h>

#include "SerializedRepository.h"

#include <atomic>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

using STI::Engine::EngineJobSourceID;
using STI::Engine::FullShotResult;
using STI::Engine::ParseID;
using STI::Engine::ParseResult;
using STI::Engine::SerializedRepository;
using STI::Engine::ShotID;
using STI::Engine::ShotResult;
using STI::Engine::ShotResultStatus;
using STI::Engine::ShotResultStatusFromString;
using STI::Engine::ShotResultStatusToString;

namespace {

std::filesystem::path makeRepositoryPath(const std::string& testName)
{
    static std::atomic<unsigned> counter{0};

    return std::filesystem::temp_directory_path()
        / ("sti3_shotresult_" + testName + "_" + std::to_string(counter++));
}

std::shared_ptr<FullShotResult> makeFullShotResult(ShotResultStatus status)
{
    EngineJobSourceID source("shotresult-test", "localhost");
    ParseID pid = ParseID::generateUniqueID(source);
    ShotID sid = ShotID::generateUniqueID(pid, source);

    auto parseResult = std::make_shared<ParseResult>();
    parseResult->pid = pid;

    auto shotResult = std::make_shared<ShotResult>();
    shotResult->sid = sid;
    shotResult->status = status;

    auto fullShotResult = std::make_shared<FullShotResult>();
    fullShotResult->parseResult = parseResult;
    fullShotResult->shotResult = shotResult;

    return fullShotResult;
}

} // namespace

TEST_CASE("ShotResult status converts to and from stable strings", "[shotresult]")
{
    CHECK(ShotResultStatusToString(ShotResultStatus::Unknown) == "Unknown");
    CHECK(ShotResultStatusToString(ShotResultStatus::Success) == "Success");
    CHECK(ShotResultStatusToString(ShotResultStatus::CompletedWithErrors) == "CompletedWithErrors");
    CHECK(ShotResultStatusToString(ShotResultStatus::CanceledByUser) == "CanceledByUser");
    CHECK(ShotResultStatusToString(ShotResultStatus::AbortedByError) == "AbortedByError");
    CHECK(ShotResultStatusToString(ShotResultStatus::AbortedByTimeout) == "AbortedByTimeout");

    CHECK(ShotResultStatusFromString("Success") == ShotResultStatus::Success);
    CHECK(ShotResultStatusFromString("CompletedWithErrors") == ShotResultStatus::CompletedWithErrors);
    CHECK(ShotResultStatusFromString("CanceledByUser") == ShotResultStatus::CanceledByUser);
    CHECK(ShotResultStatusFromString("AbortedByError") == ShotResultStatus::AbortedByError);
    CHECK(ShotResultStatusFromString("AbortedByTimeout") == ShotResultStatus::AbortedByTimeout);
    CHECK(ShotResultStatusFromString("not-a-status") == ShotResultStatus::Unknown);
}

TEST_CASE("SerializedRepository round trips ShotResult status", "[shotresult][repository]")
{
    auto fullShotResult = makeFullShotResult(ShotResultStatus::CanceledByUser);
    auto sid = fullShotResult->shotResult->sid;

    SerializedRepository repository(makeRepositoryPath("status_round_trip").string());
    REQUIRE(repository.saveShot(sid, fullShotResult));

    std::shared_ptr<ShotResult> loaded;
    REQUIRE(repository.getShotResult(sid, loaded));
    REQUIRE(loaded != nullptr);
    CHECK(loaded->status == ShotResultStatus::CanceledByUser);
}

TEST_CASE("SerializedRepository loads old ShotResult files without status", "[shotresult][repository]")
{
    auto fullShotResult = makeFullShotResult(ShotResultStatus::Success);
    auto sid = fullShotResult->shotResult->sid;

    SerializedRepository repository(makeRepositoryPath("missing_status").string());
    REQUIRE(repository.saveShot(sid, fullShotResult));

    auto paths = repository.preparePaths(sid);
    auto shotPath = std::filesystem::path(paths.experimentPath)
        / ("shot_" + sid.submissionTime.time_hh_mm_ss_mmmuuunnn() + ".xml");

    std::ifstream in(shotPath);
    REQUIRE(in.good());
    std::string xml((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

    auto start = xml.find("<status>");
    auto end = xml.find("</status>");
    REQUIRE(start != std::string::npos);
    REQUIRE(end != std::string::npos);
    end += std::string("</status>").size();
    xml.erase(start, end - start);

    std::ofstream out(shotPath, std::ios::trunc);
    REQUIRE(out.good());
    out << xml;
    out.close();

    std::shared_ptr<ShotResult> loaded;
    REQUIRE(repository.getShotResult(sid, loaded));
    REQUIRE(loaded != nullptr);
    CHECK(loaded->status == ShotResultStatus::Unknown);
}
