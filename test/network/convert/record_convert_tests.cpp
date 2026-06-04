#include <catch2/catch_test_macros.hpp>

#include "NetworkConvert.h"
#include "RemoteChannel.h"
#include "convert/Convert_Channel.h"
#include "convert/Convert_DeviceMessage.h"
#include "convert/Convert_EventEngine.h"
#include "convert/Convert_File.h"
#include "convert/Convert_Log.h"
#include "convert/Convert_Profile.h"
#include "convert/Convert_SequenceResult.h"
#include "convert/Convert_ShotResult.h"

#include <sti/device/DeviceID.h>
#include <sti/device/DeviceMessage.h>
#include <sti/device/LocalChannel.h>
#include <sti/device/LogID.h>
#include <sti/device/LogRecord.h>
#include <sti/device/Profile.h>
#include <sti/device/VersionInfo.h>
#include <sti/engine/EngineJobID.h>
#include <sti/engine/EngineJobSourceID.h>
#include <sti/engine/ParseResult.h>
#include <sti/engine/ParseID.h>
#include <sti/engine/SequenceID.h>
#include <sti/engine/ShotConfig.h>
#include <sti/engine/ShotID.h>
#include <sti/engine/ShotResult.h>
#include <sti/engine/ShotResultRecord.h>
#include <sti/engine/StackTraceData.h>
#include <sti/engine/StackTraceResult.h>
#include <sti/utils/BinaryData.h>
#include <sti/utils/FileID.h>
#include <sti/utils/MixedValue.h>
#include <sti/utils/TimeStamp.h>

#include <map>
#include <memory>
#include <set>
#include <string>

namespace
{

STI::Utils::TimeStamp makeTimeStamp(int seconds = 12)
{
    return STI::Utils::TimeStamp(2026, 5, 9, 14, 30, seconds, 123, 456, 789);
}

STI::Device::DeviceID makeDeviceID(
    const std::string& name = "camera",
    const std::string& address = "192.168.1.10",
    unsigned short module = 2)
{
    return STI::Device::DeviceID(name, address, module, "server-a");
}

STI::Utils::FileID makeFileID(
    const std::string& filename = "result.dat",
    const std::string& path = "/tmp/sti3/results")
{
    STI::Utils::FileID fileID;
    fileID.filename = filename;
    fileID.path = path;
    fileID.origin = "device-a";
    fileID.persistenceLocation = "local-cache";
    fileID.creationTime = makeTimeStamp();
    return fileID;
}

STI::Engine::EngineJobSourceID makeSourceID()
{
    return STI::Engine::EngineJobSourceID("operator", "control-host");
}

void checkDeviceID(const STI::Device::DeviceID& actual, const STI::Device::DeviceID& expected)
{
    CHECK(actual.getName() == expected.getName());
    CHECK(actual.getAddress() == expected.getAddress());
    CHECK(actual.getModule() == expected.getModule());
    CHECK(actual.getTargetServerID() == expected.getTargetServerID());
    CHECK(actual.getID() == expected.getID());
}

void checkFileID(const STI::Utils::FileID& actual, const STI::Utils::FileID& expected)
{
    CHECK(actual.filename == expected.filename);
    CHECK(actual.path == expected.path);
    CHECK(actual.origin == expected.origin);
    CHECK(actual.persistenceLocation == expected.persistenceLocation);
    CHECK(actual.creationTime == expected.creationTime);
}

void checkSourceID(
    const STI::Engine::EngineJobSourceID& actual,
    const STI::Engine::EngineJobSourceID& expected)
{
    CHECK(actual.user == expected.user);
    CHECK(actual.machine == expected.machine);
}

void checkSequenceID(const STI::Engine::SequenceID& actual, const STI::Engine::SequenceID& expected)
{
    CHECK(actual.timestamp == expected.timestamp);
    checkSourceID(actual.jobSourceID, expected.jobSourceID);
}

void checkParseID(const STI::Engine::ParseID& actual, const STI::Engine::ParseID& expected)
{
    CHECK(actual.parseTimestamp == expected.parseTimestamp);
    CHECK(actual.shotType == expected.shotType);
    checkSourceID(actual.jobSourceID, expected.jobSourceID);

    if (expected.shotType == STI::Engine::ShotType::SequenceEntry) {
        checkSequenceID(actual.sequenceEntryID.seqID, expected.sequenceEntryID.seqID);
        CHECK(actual.sequenceEntryID.seqIndex == expected.sequenceEntryID.seqIndex);
    }
}

void checkShotID(const STI::Engine::ShotID& actual, const STI::Engine::ShotID& expected)
{
    checkParseID(actual.parseID, expected.parseID);
    checkSourceID(actual.jobSourceID, expected.jobSourceID);
    CHECK(actual.submissionTime == expected.submissionTime);
}

void checkLogID(const STI::Device::LogID& actual, const STI::Device::LogID& expected)
{
    checkDeviceID(actual.deviceID, expected.deviceID);
    CHECK(actual.date == expected.date);
    CHECK(actual.logName == expected.logName);
    CHECK(actual.index == expected.index);
}

void checkLogFileRecord(
    const STI::Device::LogFileRecord& actual,
    const STI::Device::LogFileRecord& expected)
{
    checkLogID(actual.id, expected.id);
    checkFileID(actual.fileID, expected.fileID);
    CHECK(actual.bytes == expected.bytes);
    CHECK(actual.lineCount == expected.lineCount);
    CHECK(actual.firstEntryTime == expected.firstEntryTime);
    CHECK(actual.lastEntryTime == expected.lastEntryTime);
}

STI::Device::LogFileRecord makeLogFileRecord(unsigned index)
{
    STI::Device::LogFileRecord record;
    record.id = STI::Device::LogID(makeDeviceID(), "2026-05-09", "events", index);
    record.fileID = makeFileID("events-" + std::to_string(index) + ".log", "/tmp/sti3/logs");
    record.bytes = 1024 + index;
    record.lineCount = 50 + index;
    record.firstEntryTime = makeTimeStamp(static_cast<int>(index));
    record.lastEntryTime = makeTimeStamp(static_cast<int>(index + 10));
    return record;
}

} // namespace

TEST_CASE("NetworkConvert: DeviceID, VersionInfo, and FileID round trip")
{
    const auto deviceID = makeDeviceID();
    auto tDeviceID = STI::Network::convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(deviceID);
    auto deviceIDRoundTrip = STI::Network::convert<STI::TNetwork::TDeviceID, STI::Device::DeviceID>(tDeviceID);
    checkDeviceID(deviceIDRoundTrip, deviceID);

    STI::Device::VersionInfo version;
    version.component = "stinetwork";
    version.version = "3.2.1";
    version.major = 3;
    version.minor = 2;
    version.patch = 1;
    version.buildNumber = 42;
    version.buildString = "debug";
    version.gitCommit = "abcdef123456";
    version.gitDirty = true;
    version.metadata = {{"compiler", "gcc"}, {"platform", "linux"}};

    auto tVersion = STI::Network::convert<STI::Device::VersionInfo, STI::TNetwork::TVersionInfo>(version);
    auto versionRoundTrip = STI::Network::convert<STI::TNetwork::TVersionInfo, STI::Device::VersionInfo>(tVersion);
    CHECK(versionRoundTrip.component == version.component);
    CHECK(versionRoundTrip.version == version.version);
    CHECK(versionRoundTrip.major == version.major);
    CHECK(versionRoundTrip.minor == version.minor);
    CHECK(versionRoundTrip.patch == version.patch);
    CHECK(versionRoundTrip.buildNumber == version.buildNumber);
    CHECK(versionRoundTrip.buildString == version.buildString);
    CHECK(versionRoundTrip.gitCommit == version.gitCommit);
    CHECK(versionRoundTrip.gitDirty == version.gitDirty);
    CHECK(versionRoundTrip.metadata == version.metadata);

    const auto fileID = makeFileID();
    auto tFileID = STI::Network::convert<STI::Utils::FileID, STI::TNetwork::TFileID>(fileID);
    auto fileIDRoundTrip = STI::Network::convert<STI::TNetwork::TFileID, STI::Utils::FileID>(tFileID);
    checkFileID(fileIDRoundTrip, fileID);
}

TEST_CASE("NetworkConvert: Profile round trips attributes and channel values")
{
    STI::Device::Profile profile("experiment");
    profile.type = STI::Device::ProfileType::All;
    profile.readOnly = true;
    profile.attributeData = {{"mode", "scan"}, {"operator", "user-a"}};
    profile.channelData = {
        {1, STI::Utils::MixedValue(3.25)},
        {2, STI::Utils::MixedValue("armed")},
    };

    STI::TNetwork::TProfile tProfile;
    REQUIRE(STI::Network::convert<STI::Device::Profile, STI::TNetwork::TProfile>(profile, tProfile));

    STI::Device::Profile roundTrip;
    REQUIRE(STI::Network::convert<STI::TNetwork::TProfile, STI::Device::Profile>(tProfile, roundTrip));

    CHECK(roundTrip.name == profile.name);
    CHECK(roundTrip.type == profile.type);
    CHECK(roundTrip.readOnly == profile.readOnly);
    CHECK(roundTrip.attributeData == profile.attributeData);
    REQUIRE(roundTrip.channelData.size() == profile.channelData.size());
    CHECK(roundTrip.channelData.at(1).getDouble() == profile.channelData.at(1).getDouble());
    CHECK(roundTrip.channelData.at(2).getString() == profile.channelData.at(2).getString());
}

TEST_CASE("NetworkConvert: Channel snapshot round trips last value and measurement", "[network][convert][channel]")
{
    auto channel = std::make_shared<STI::Device::LocalChannel>(
        4, STI::Device::ChannelType::Input, STI::Utils::MixedValueType::Double, STI::Utils::MixedValueType::Int, "input");
    channel->saveLastValue(STI::Utils::MixedValue(7));
    channel->saveLastMeasurement(STI::Utils::MixedValue(3.5));

    STI::TNetwork::TChannel tChannel;
    REQUIRE(STI::Network::convert<std::shared_ptr<STI::Device::Channel>, STI::TNetwork::TChannel>(channel, tChannel));

    auto remote = STI::Network::convert<STI::TNetwork::TChannel, std::shared_ptr<STI::Network::RemoteChannel>>(tChannel);
    REQUIRE(remote != nullptr);
    CHECK(remote->getLastValue() == STI::Utils::MixedValue(7));
    CHECK(remote->getLastMeasurement() == STI::Utils::MixedValue(3.5));
}

TEST_CASE("NetworkConvert: Channel snapshot suppresses heavy last measurement payloads", "[network][convert][channel]")
{
    auto channel = std::make_shared<STI::Device::LocalChannel>(
        5, STI::Device::ChannelType::Input, STI::Utils::MixedValueType::Binary, STI::Utils::MixedValueType::Empty, "binary");
    channel->saveLastMeasurement(STI::Utils::MixedValue(std::make_shared<STI::Utils::BinaryData>()));

    STI::TNetwork::TChannel tChannel;
    REQUIRE(STI::Network::convert<std::shared_ptr<STI::Device::Channel>, STI::TNetwork::TChannel>(channel, tChannel));

    auto remote = STI::Network::convert<STI::TNetwork::TChannel, std::shared_ptr<STI::Network::RemoteChannel>>(tChannel);
    REQUIRE(remote != nullptr);
    CHECK(remote->getLastMeasurement().isEmpty());
}

TEST_CASE("NetworkConvert: ChannelUpdateMessage round trips channel and measurement maps", "[network][convert][channel]")
{
    auto message = std::make_shared<STI::Device::ChannelUpdateMessage>(makeDeviceID(), 1, STI::Utils::MixedValue(11));
    message->channelValues[2] = STI::Utils::MixedValue(22);
    message->measurementValues[1] = STI::Utils::MixedValue(1.5);
    message->measurementValues[3] = STI::Utils::MixedValue("done");

    STI::TNetwork::TChannelUpdateMessage tMessage;
    REQUIRE(STI::Network::convert<std::shared_ptr<STI::Device::DeviceMessage>, STI::TNetwork::TDeviceMessage>(
        std::static_pointer_cast<STI::Device::DeviceMessage>(message), tMessage.base));
    REQUIRE(STI::Network::convert<std::shared_ptr<STI::Device::ChannelUpdateMessage>, STI::TNetwork::TChannelUpdateMessage>(
        message, tMessage));

    std::shared_ptr<STI::Device::ChannelUpdateMessage> roundTrip;
    REQUIRE(STI::Network::convert<STI::TNetwork::TChannelUpdateMessage, std::shared_ptr<STI::Device::ChannelUpdateMessage>>(
        tMessage, roundTrip));
    REQUIRE(roundTrip != nullptr);

    REQUIRE(roundTrip->channelValues.size() == 2);
    CHECK(roundTrip->channelValues.at(1) == STI::Utils::MixedValue(11));
    CHECK(roundTrip->channelValues.at(2) == STI::Utils::MixedValue(22));

    REQUIRE(roundTrip->measurementValues.size() == 2);
    CHECK(roundTrip->measurementValues.at(1) == STI::Utils::MixedValue(1.5));
    CHECK(roundTrip->measurementValues.at(3) == STI::Utils::MixedValue("done"));
}

TEST_CASE("NetworkConvert: log records round trip nested file records")
{
    const auto fileRecord0 = makeLogFileRecord(0);
    const auto fileRecord1 = makeLogFileRecord(1);

    STI::Device::LogNameRecord logNameRecord;
    logNameRecord.logName = "events";
    logNameRecord.files = {{0, fileRecord0}, {1, fileRecord1}};
    logNameRecord.totalBytes = fileRecord0.bytes + fileRecord1.bytes;
    logNameRecord.totalLines = fileRecord0.lineCount + fileRecord1.lineCount;
    logNameRecord.nextIndex = 2;
    logNameRecord.lastUpdate = makeTimeStamp(45);

    STI::TNetwork::TLogNameRecord tLogNameRecord;
    REQUIRE(STI::Network::convert<STI::Device::LogNameRecord, STI::TNetwork::TLogNameRecord>(
        logNameRecord, tLogNameRecord));

    STI::Device::LogNameRecord logNameRoundTrip;
    REQUIRE(STI::Network::convert<STI::TNetwork::TLogNameRecord, STI::Device::LogNameRecord>(
        tLogNameRecord, logNameRoundTrip));

    CHECK(logNameRoundTrip.logName == logNameRecord.logName);
    CHECK(logNameRoundTrip.totalBytes == logNameRecord.totalBytes);
    CHECK(logNameRoundTrip.totalLines == logNameRecord.totalLines);
    CHECK(logNameRoundTrip.nextIndex == logNameRecord.nextIndex);
    CHECK(logNameRoundTrip.lastUpdate == logNameRecord.lastUpdate);
    REQUIRE(logNameRoundTrip.files.size() == 2);
    checkLogFileRecord(logNameRoundTrip.files.at(0), fileRecord0);
    checkLogFileRecord(logNameRoundTrip.files.at(1), fileRecord1);

    STI::Device::DeviceLogRecord deviceRecord;
    deviceRecord.deviceID = makeDeviceID().getID();
    deviceRecord.status = STI::Device::LogRecordStatus::LogsPresent;
    deviceRecord.logs = {{logNameRecord.logName, logNameRecord}};

    STI::Device::LogRecord logRecord;
    logRecord.timeStamp = makeTimeStamp(55);
    logRecord.deviceLogRecords = {{deviceRecord.deviceID, deviceRecord}};

    STI::TNetwork::TLogRecord tLogRecord;
    REQUIRE(STI::Network::convert<STI::Device::LogRecord, STI::TNetwork::TLogRecord>(logRecord, tLogRecord));

    STI::Device::LogRecord roundTrip;
    REQUIRE(STI::Network::convert<STI::TNetwork::TLogRecord, STI::Device::LogRecord>(tLogRecord, roundTrip));

    CHECK(roundTrip.timeStamp == logRecord.timeStamp);
    REQUIRE(roundTrip.deviceLogRecords.size() == 1);
    const auto& roundTripDevice = roundTrip.deviceLogRecords.at(deviceRecord.deviceID);
    CHECK(roundTripDevice.status == deviceRecord.status);
    CHECK(roundTripDevice.logNames == std::set<std::string>{"events"});
    REQUIRE(roundTripDevice.logs.size() == 1);
    CHECK(roundTripDevice.logs.at("events").totalBytes == logNameRecord.totalBytes);
}

TEST_CASE("NetworkConvert: sequence, parse, and shot IDs round trip")
{
    const auto sourceID = makeSourceID();
    const STI::Engine::SequenceID sequenceID(makeTimeStamp(10), sourceID);
    const STI::Engine::SequenceIndex sequenceIndex(4, 2);
    const STI::Engine::SequenceEntryID sequenceEntryID(sequenceID, sequenceIndex);
    const STI::Engine::ParseID parseID(makeTimeStamp(20), sourceID, sequenceEntryID);
    const STI::Engine::ShotID shotID(parseID, sourceID, makeTimeStamp(30));

    auto tSourceID = STI::Network::convert<STI::Engine::EngineJobSourceID, STI::TNetwork::TEngineJobSourceID>(
        sourceID);
    checkSourceID(
        STI::Network::convert<STI::TNetwork::TEngineJobSourceID, STI::Engine::EngineJobSourceID>(tSourceID),
        sourceID);

    auto tSequenceIndex =
        STI::Network::convert<STI::Engine::SequenceIndex, STI::TNetwork::TSequenceIndex>(sequenceIndex);
    CHECK(STI::Network::convert<STI::TNetwork::TSequenceIndex, STI::Engine::SequenceIndex>(tSequenceIndex)
          == sequenceIndex);

    auto tSequenceID = STI::Network::convert<STI::Engine::SequenceID, STI::TNetwork::TSequenceID>(sequenceID);
    checkSequenceID(
        STI::Network::convert<STI::TNetwork::TSequenceID, STI::Engine::SequenceID>(tSequenceID),
        sequenceID);

    auto tSequenceEntryID =
        STI::Network::convert<STI::Engine::SequenceEntryID, STI::TNetwork::TSequenceEntryID>(sequenceEntryID);
    auto sequenceEntryRoundTrip =
        STI::Network::convert<STI::TNetwork::TSequenceEntryID, STI::Engine::SequenceEntryID>(tSequenceEntryID);
    checkSequenceID(sequenceEntryRoundTrip.seqID, sequenceEntryID.seqID);
    CHECK(sequenceEntryRoundTrip.seqIndex == sequenceEntryID.seqIndex);

    auto tParseID = STI::Network::convert<STI::Engine::ParseID, STI::TNetwork::TParseID>(parseID);
    checkParseID(STI::Network::convert<STI::TNetwork::TParseID, STI::Engine::ParseID>(tParseID), parseID);

    auto tShotID = STI::Network::convert<STI::Engine::ShotID, STI::TNetwork::TShotID>(shotID);
    checkShotID(STI::Network::convert<STI::TNetwork::TShotID, STI::Engine::ShotID>(tShotID), shotID);
}

TEST_CASE("NetworkConvert: ShotConfig and ShotResultRecord round trip")
{
    const auto sourceID = makeSourceID();
    const STI::Engine::ShotConfig shotConfig(
        STI::Engine::ShotType::SequenceEntry, sourceID, 7, "sequence.py", "entry 4");

    auto tShotConfig = STI::Network::convert<STI::Engine::ShotConfig, STI::TNetwork::TShotConfig>(shotConfig);
    auto shotConfigRoundTrip =
        STI::Network::convert<STI::TNetwork::TShotConfig, STI::Engine::ShotConfig>(tShotConfig);

    CHECK(shotConfigRoundTrip.shotType == shotConfig.shotType);
    checkSourceID(shotConfigRoundTrip.jobSourceID, shotConfig.jobSourceID);
    CHECK(shotConfigRoundTrip.targetEnginePool == shotConfig.targetEnginePool);
    CHECK(shotConfigRoundTrip.file == shotConfig.file);
    CHECK(shotConfigRoundTrip.comment == shotConfig.comment);

    STI::Engine::ShotResultRecord root(makeDeviceID("root", "host-a", 1));
    root.recordStatus = STI::Engine::RecordStatus::MissingResults;
    root.dependencies.emplace_back(makeDeviceID("child-a", "host-a", 2));
    root.dependencies.back().recordStatus = STI::Engine::RecordStatus::Complete;
    root.dependencies.emplace_back(makeDeviceID("child-b", "host-b", 1));
    root.dependencies.back().recordStatus = STI::Engine::RecordStatus::MissingDevice;

    auto tRecord = STI::Network::convert<STI::Engine::ShotResultRecord, STI::TNetwork::TShotResultRecord>(root);
    auto roundTrip = STI::Network::convert<STI::TNetwork::TShotResultRecord, STI::Engine::ShotResultRecord>(
        tRecord);

    checkDeviceID(roundTrip.deviceID, root.deviceID);
    CHECK(roundTrip.recordStatus == root.recordStatus);
    REQUIRE(roundTrip.dependencies.size() == root.dependencies.size());
    checkDeviceID(roundTrip.dependencies.at(0).deviceID, root.dependencies.at(0).deviceID);
    CHECK(roundTrip.dependencies.at(0).recordStatus == root.dependencies.at(0).recordStatus);
    checkDeviceID(roundTrip.dependencies.at(1).deviceID, root.dependencies.at(1).deviceID);
    CHECK(roundTrip.dependencies.at(1).recordStatus == root.dependencies.at(1).recordStatus);
}

TEST_CASE("NetworkConvert: ParseResult and ShotResult carry jobOwner")
{
    const auto sourceID = makeSourceID();
    const auto jobOwner = makeDeviceID("server", "control-host", 0);
    const auto parseID = STI::Engine::ParseID::generateUniqueID(sourceID);
    const auto shotID = STI::Engine::ShotID::generateUniqueID(parseID, sourceID);

    STI::Engine::ParseResult parseResult;
    parseResult.pid = parseID;
    parseResult.shotConfig = STI::Engine::ShotConfig(
        STI::Engine::ShotType::Single, sourceID, 3, "shot.py", "job owner test");
    parseResult.jobOwner = jobOwner;
    parseResult.stackTraceResult = std::make_shared<STI::Engine::StackTraceResult>(parseID);
    parseResult.stackTraceResult->stackTraceData = std::make_shared<STI::Engine::StackTraceData>();

    STI::TNetwork::TParseResult tParseResult;
    REQUIRE(STI::Network::convert<STI::Engine::ParseResult, STI::TNetwork::TParseResult>(
        parseResult, tParseResult));

    STI::Engine::ParseResult parseRoundTrip;
    REQUIRE(STI::Network::convert<STI::TNetwork::TParseResult, STI::Engine::ParseResult>(
        tParseResult, parseRoundTrip));
    checkDeviceID(parseRoundTrip.jobOwner, jobOwner);

    auto shotResult = std::make_shared<STI::Engine::ShotResult>();
    shotResult->sid = shotID;
    shotResult->playTime = makeTimeStamp(40);
    shotResult->jobOwner = jobOwner;
    shotResult->status = STI::Engine::ShotResultStatus::Success;
    shotResult->shotResultRecord.deviceID = makeDeviceID("device", "experiment-host", 2);

    STI::TNetwork::TShotResult tShotResult;
    REQUIRE(STI::Network::convert<std::shared_ptr<STI::Engine::ShotResult>, STI::TNetwork::TShotResult>(
        shotResult, tShotResult));

    std::shared_ptr<STI::Engine::ShotResult> shotRoundTrip;
    REQUIRE(STI::Network::convert<STI::TNetwork::TShotResult, std::shared_ptr<STI::Engine::ShotResult>>(
        tShotResult, shotRoundTrip));
    REQUIRE(shotRoundTrip != nullptr);
    checkDeviceID(shotRoundTrip->jobOwner, jobOwner);
}
