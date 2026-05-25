#include <catch2/catch_test_macros.hpp>

#include <sti/engine/EngineJobSourceID.h>
#include <sti/engine/FullShotResult.h>
#include <sti/engine/ParseID.h>
#include <sti/engine/ParseResult.h>
#include <sti/engine/ShotID.h>
#include <sti/engine/ShotResult.h>
#include <sti/device/DeviceID.h>
#include <sti/device/VersionInfo.h>

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
using STI::Device::DeviceID;
using STI::Device::VersionInfo;

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
    DeviceID jobOwner("job-owner", "localhost", 9);

    auto parseResult = std::make_shared<ParseResult>();
    parseResult->pid = pid;
    parseResult->jobOwner = jobOwner;

    auto shotResult = std::make_shared<ShotResult>();
    shotResult->sid = sid;
    shotResult->jobOwner = jobOwner;
    shotResult->status = status;
    shotResult->shotResultRecord.deviceID = jobOwner;

    auto fullShotResult = std::make_shared<FullShotResult>();
    fullShotResult->parseResult = parseResult;
    fullShotResult->shotResult = shotResult;

    return fullShotResult;
}

void checkDeviceID(const DeviceID& actual, const DeviceID& expected)
{
    CHECK(actual.getName() == expected.getName());
    CHECK(actual.getAddress() == expected.getAddress());
    CHECK(actual.getModule() == expected.getModule());
    CHECK(actual.getTargetServerID() == expected.getTargetServerID());
    CHECK(actual.getID() == expected.getID());
}

bool eraseXmlElement(std::string& xml, const std::string& elementName)
{
    auto start = xml.find("<" + elementName + ">");
    auto end = xml.find("</" + elementName + ">");
    if (start == std::string::npos || end == std::string::npos) {
        return false;
    }

    end += elementName.size() + 3;
    xml.erase(start, end - start);
    return true;
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

TEST_CASE("SerializedRepository round trips result job owners", "[shotresult][parseresult][repository]")
{
    auto fullShotResult = makeFullShotResult(ShotResultStatus::Success);
    auto pid = fullShotResult->parseResult->pid;
    auto sid = fullShotResult->shotResult->sid;
    auto expectedOwner = fullShotResult->shotResult->jobOwner;

    SerializedRepository repository(makeRepositoryPath("job_owner_round_trip").string());
    REQUIRE(repository.saveShot(sid, fullShotResult));

    std::shared_ptr<ParseResult> loadedParse;
    REQUIRE(repository.getParseResult(pid, loadedParse));
    REQUIRE(loadedParse != nullptr);
    checkDeviceID(loadedParse->jobOwner, expectedOwner);

    std::shared_ptr<ShotResult> loadedShot;
    REQUIRE(repository.getShotResult(sid, loadedShot));
    REQUIRE(loadedShot != nullptr);
    checkDeviceID(loadedShot->jobOwner, expectedOwner);
}

TEST_CASE("SerializedRepository round trips ShotResult device versions", "[shotresult] [repository] [version]")
{
    auto fullShotResult = makeFullShotResult(ShotResultStatus::Success);
    auto sid = fullShotResult->shotResult->sid;

    DeviceID deviceID("test-device", "localhost", 1);
    VersionInfo libraryVersion("sti3", "3.1.0-test");
    libraryVersion.buildNumber = 12;
    libraryVersion.buildString = "test-build";
    libraryVersion.gitCommit = "abcdef";
    libraryVersion.gitDirty = true;
    libraryVersion.metadata["package"] = "test";

    VersionInfo deviceVersion("test-device", "0.2.0");
    fullShotResult->shotResult->versions[deviceID] = {libraryVersion, deviceVersion};

    SerializedRepository repository(makeRepositoryPath("versions_round_trip").string());
    REQUIRE(repository.saveShot(sid, fullShotResult));

    std::shared_ptr<ShotResult> loaded;
    REQUIRE(repository.getShotResult(sid, loaded));
    REQUIRE(loaded != nullptr);

    auto versionsIt = loaded->versions.find(deviceID);
    REQUIRE(versionsIt != loaded->versions.end());
    REQUIRE(versionsIt->second.size() == 2);
    CHECK(versionsIt->second[0].component == "sti3");
    CHECK(versionsIt->second[0].version == "3.1.0-test");
    CHECK(versionsIt->second[0].buildNumber == 12);
    CHECK(versionsIt->second[0].metadata.at("package") == "test");
    CHECK(versionsIt->second[1].component == "test-device");
    CHECK(versionsIt->second[1].version == "0.2.0");
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

TEST_CASE("SerializedRepository loads old result files without jobOwner", "[shotresult][parseresult][repository]")
{
    auto fullShotResult = makeFullShotResult(ShotResultStatus::Success);
    auto pid = fullShotResult->parseResult->pid;
    auto sid = fullShotResult->shotResult->sid;

    SerializedRepository repository(makeRepositoryPath("missing_job_owner").string());
    REQUIRE(repository.saveShot(sid, fullShotResult));

    auto parsePaths = repository.preparePaths(pid);
    auto parsePath = std::filesystem::path(parsePaths.experimentPath)
        / ("parse_" + pid.parseTimestamp.time_hh_mm_ss_mmmuuunnn() + ".xml");

    std::ifstream parseIn(parsePath);
    REQUIRE(parseIn.good());
    std::string parseXml((std::istreambuf_iterator<char>(parseIn)), std::istreambuf_iterator<char>());
    REQUIRE(eraseXmlElement(parseXml, "jobOwner"));

    std::ofstream parseOut(parsePath, std::ios::trunc);
    REQUIRE(parseOut.good());
    parseOut << parseXml;
    parseOut.close();

    auto shotPaths = repository.preparePaths(sid);
    auto shotPath = std::filesystem::path(shotPaths.experimentPath)
        / ("shot_" + sid.submissionTime.time_hh_mm_ss_mmmuuunnn() + ".xml");

    std::ifstream shotIn(shotPath);
    REQUIRE(shotIn.good());
    std::string shotXml((std::istreambuf_iterator<char>(shotIn)), std::istreambuf_iterator<char>());
    REQUIRE(eraseXmlElement(shotXml, "jobOwner"));

    std::ofstream shotOut(shotPath, std::ios::trunc);
    REQUIRE(shotOut.good());
    shotOut << shotXml;
    shotOut.close();

    std::shared_ptr<ParseResult> loadedParse;
    REQUIRE(repository.getParseResult(pid, loadedParse));
    REQUIRE(loadedParse != nullptr);
    CHECK(loadedParse->jobOwner.empty());

    std::shared_ptr<ShotResult> loadedShot;
    REQUIRE(repository.getShotResult(sid, loadedShot));
    REQUIRE(loadedShot != nullptr);
    checkDeviceID(loadedShot->jobOwner, loadedShot->shotResultRecord.deviceID);
}
