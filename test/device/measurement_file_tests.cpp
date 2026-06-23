#include <catch2/catch_test_macros.hpp>

#include <sti/device/ChannelManager.h>
#include <sti/device/LocalChannel.h>
#include <sti/device/PersistenceManager.h>
#include <sti/engine/DeviceEventParser.h>
#include <sti/engine/EngineID.h>
#include <sti/engine/EngineJobSourceID.h>
#include <sti/engine/EngineParsingMessage.h>
#include <sti/engine/FullShotResult.h>
#include <sti/engine/Measurement.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/RawEventGroup.h>
#include <sti/engine/RawEventTarget.h>
#include <sti/engine/ResultsCollector.h>
#include <sti/engine/SequenceResult.h>
#include <sti/engine/ShotID.h>
#include <sti/engine/ShotResult.h>
#include <sti/engine/SynchronousEvent.h>
#include <sti/utils/BinaryData.h>
#include <sti/utils/BinaryDataStream.h>
#include <sti/utils/FileHolder.h>
#include <sti/utils/Image.h>
#include <sti/utils/LocalFileHolder.h>
#include <sti/utils/VirtualFileHolder.h>
#include <sti/utils/VirtualFileServer.h>

#include "EventEngineParser.h"
#include "LegacyExperimentXMLBuilder.h"
#include "LocalResultsCollector.h"
#include "fileholder_tests_support.h"

#include <tinyxml2.h>

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

using fileholder_test_support::TempDir;
using fileholder_test_support::makeFileID;
using fileholder_test_support::readFileToString;

using STI::Device::Channel;
using STI::Device::ChannelManager;
using STI::Device::ChannelType;
using STI::Device::DeviceID;
using STI::Device::ImportedFile;
using STI::Device::ImportFileOptions;
using STI::Device::LocalChannel;
using STI::Device::PersistenceManager;
using STI::Engine::DeviceEventParser;
using STI::Engine::EngineID;
using STI::Engine::EngineJobSourceID;
using STI::Engine::EngineJobStatus;
using STI::Engine::EventEngineParser;
using STI::Engine::FullShotResult;
using STI::Engine::LegacyExperimentXMLBuilder;
using STI::Engine::LocalResultsCollector;
using STI::Engine::Measurement;
using STI::Engine::MeasurementMap;
using STI::Engine::MeasurementVector;
using STI::Engine::ParseID;
using STI::Engine::ParseResult;
using STI::Engine::RawEventGroup;
using STI::Engine::RawEventMap;
using STI::Engine::RawEventTarget;
using STI::Engine::RawEventType;
using STI::Engine::ResultsCollector;
using STI::Engine::ResultsCollectorFactory;
using STI::Engine::ResultsPaths;
using STI::Engine::SequenceEntryID;
using STI::Engine::SequenceID;
using STI::Engine::SequenceResult;
using STI::Engine::ShotID;
using STI::Engine::ShotResult;
using STI::Engine::ShotResultRecord;
using STI::Engine::SynchronousEvent;
using STI::Engine::SynchronousEventVector;
using STI::Utils::FileHolder;
using STI::Utils::FileHolderFactory;
using STI::Utils::FileID;
using STI::Utils::FileServer;
using STI::Utils::BinaryData;
using STI::Utils::BinaryDataStream;
using STI::Utils::BinaryDataStreamTarget;
using STI::Utils::Image;
using STI::Utils::LocalFileHolderFactory;
using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;
using STI::Utils::VirtualFileHolder;
using STI::Utils::VirtualFileServer;
using STI::Utils::VirtualFileServerFactory;

namespace {

class SingleInputChannelManager : public ChannelManager
{
public:
    SingleInputChannelManager(unsigned short channelNumber)
        : channel(std::make_shared<LocalChannel>(channelNumber,
                                                 ChannelType::Input,
                                                 MixedValueType::Any,
                                                 MixedValueType::Empty,
                                                 "file input"))
    {
    }

    void getChannels(std::vector<std::shared_ptr<Channel>>& channels) override
    {
        channels = {channel};
    }

    bool getChannel(short channelNumber, std::shared_ptr<Channel>& out) override
    {
        if (channelNumber == channel->getChannelNumber()) {
            out = channel;
            return true;
        }
        return false;
    }

    bool writeChannel(short, const MixedValue&) override { return false; }
    bool readChannel(short, const MixedValue&, MixedValue&) override { return false; }
    void stop() override {}

private:
    std::shared_ptr<LocalChannel> channel;
};

class TestPersistenceManager : public PersistenceManager
{
public:
    explicit TestPersistenceManager(const std::string& originID)
        : holderFactory(originID)
    {
    }

    bool findShot(const ShotID&) override { return false; }
    bool getParseResult(const ParseID&, std::shared_ptr<ParseResult>&) override { return false; }
    bool getShotResult(const ShotID&, std::shared_ptr<ShotResult>&) override { return false; }
    bool getSequenceResult(const SequenceID&, std::shared_ptr<SequenceResult>&) override { return false; }
    bool saveShot(const ShotID&, const std::shared_ptr<FullShotResult>&, bool) override { return false; }
    ShotResultRecord transferResults(const std::shared_ptr<ResultsCollector>&) override { return ShotResultRecord(); }
    void setResultsCollectorFactory(const std::shared_ptr<ResultsCollectorFactory>&) override {}
    bool getMeasurements(const ShotID&, std::shared_ptr<MeasurementMap>&) override { return false; }
    void setFileHolderFactory(const std::shared_ptr<FileHolderFactory>&) override {}
    void setVirtualFileServerFactory(const std::shared_ptr<VirtualFileServerFactory>&) override {}
    void setFileServer(const std::shared_ptr<FileServer>& server) override { fileServer = server; }
    bool getFileServer(std::shared_ptr<FileServer>& server) override
    {
        server = fileServer;
        return server != nullptr;
    }

    std::shared_ptr<VirtualFileServer> makeVirtualFileServer() override
    {
        return std::make_shared<VirtualFileServer>();
    }

    std::shared_ptr<ImportedFile> importFile(
        const FileID&,
        const std::shared_ptr<FileServer>&,
        const ImportFileOptions& = ImportFileOptions()) override { return nullptr; }
    bool releaseImportedFile(const std::string&) override { return false; }

    std::string getBasePath() const override { return ""; }
    std::string getTemporaryPath() const override { return ""; }

    void addSequence(const std::shared_ptr<SequenceResult>&) override {}
    bool updateSequence(const SequenceEntryID&, const ShotID&, const EngineJobStatus&, bool) override { return false; }
    bool saveSequence(const std::shared_ptr<SequenceResult>&, bool) override { return false; }

    std::shared_ptr<FileHolder> makeFileHolder(const std::string& path, const std::string& filename) override
    {
        return holderFactory.makeFileHolder(path, filename);
    }

    std::shared_ptr<FileHolder> makeVirtualFileHolder(const FileID& fileID) override
    {
        return holderFactory.makeVirtualFileHolder(fileID);
    }

    std::shared_ptr<FileHolder> makeVirtualFileHolder(const std::shared_ptr<VirtualFileHolder>& backingHolder) override
    {
        return holderFactory.makeVirtualFileHolder(backingHolder);
    }

private:
    LocalFileHolderFactory holderFactory;
    std::shared_ptr<FileServer> fileServer;
};

class FileMeasurementEvent : public SynchronousEvent
{
public:
    explicit FileMeasurementEvent(double time)
        : SynchronousEvent(time)
    {
    }

    void loadEvent() override {}
    void playEvent() override {}
    void collectMeasurementData() override {}
    void stopEvent() override {}
    void pauseEvent() override {}
    void unpauseEvent(bool) override {}
};

class FileMeasurementParser : public DeviceEventParser
{
public:
    explicit FileMeasurementParser(const DeviceID& deviceID)
        : deviceID(deviceID)
    {
    }

    bool isEventTarget(const DeviceID& id) override
    {
        return id == deviceID;
    }

    void getEventTargets(std::set<DeviceID>& targetIDs) override
    {
        targetIDs.insert(deviceID);
    }

    double getMinimumEventSpacing() override { return 1.0; }
    double getMinimumEventStartTime() override { return 0.0; }

    void parseEvents(const RawEventMap& events, SynchronousEventVector& synchedEvents) override
    {
        for (const auto& eventGroup : events) {
            auto event = std::make_shared<FileMeasurementEvent>(eventGroup.first);
            for (const auto& rawEvent : eventGroup.second) {
                event->addMeasurement(rawEvent);
            }
            synchedEvents.push_back(event);
        }
    }

private:
    DeviceID deviceID;
};

class PayloadStream : public BinaryDataStream
{
public:
    explicit PayloadStream(std::string payload)
        : payload(std::move(payload))
    {
    }

    void transfer(const std::shared_ptr<BinaryDataStreamTarget>& target) override
    {
        ++transferCount;
        if (target == nullptr) {
            return;
        }

        auto chunk = std::make_shared<BinaryData>();
        auto* buffer = new char[payload.size()];
        std::memcpy(buffer, payload.data(), payload.size());
        chunk->assign(buffer, payload.size());

        target->start();
        target->writeNext(chunk);
        target->stop();
    }

    std::string payload;
    unsigned transferCount{0};
};

class DummyFileServer : public FileServer
{
public:
    bool addFile(const std::shared_ptr<FileHolder>&) override { return false; }
    bool findFile(const FileID&) override { return false; }
    int getFileSize(const FileID&) override { return 0; }
    bool transferFile(const FileID&, const std::shared_ptr<FileHolder>&, STI::Utils::FileTransferType) override { return false; }
    bool transferFilePartial(const FileID&, const std::shared_ptr<FileHolder>&, int, int) override { return false; }
    bool deleteFile(const FileID&) override { return false; }
};

ShotID makeShotID()
{
    EngineJobSourceID source("measurement-file-test", "localhost");
    auto parseID = ParseID::generateUniqueID(source);
    return ShotID::generateUniqueID(parseID, source);
}

} // namespace

TEST_CASE("Measurement FileID results transfer attached virtual files and document filenames", "[measurement][file]")
{
    constexpr unsigned short channelNumber = 3;
    const DeviceID deviceID("MeasurementDevice", "127.0.0.1", 1);

    auto channelManager = std::make_shared<SingleInputChannelManager>(channelNumber);
    auto persistenceManager = std::make_shared<TestPersistenceManager>("collector");
    FileMeasurementParser deviceParser(deviceID);
    EventEngineParser parser(EngineID(0), deviceID, channelManager, persistenceManager, &deviceParser);

    RawEventGroup group("root", "");
    group.addEvent(RawEventTarget(deviceID, channelNumber), 10.0, MixedValue(), RawEventType::Measurement);

    SynchronousEventVector synchedEvents;
    auto parseID = ParseID::generateUniqueID(EngineJobSourceID("measurement-file-test", "localhost"));
    REQUIRE(parser.parse(group, synchedEvents, parseID));
    REQUIRE(synchedEvents.size() == 1);

    auto originalFileID = makeFileID(deviceID, "virtual/path", "measurement.txt");
    auto virtualFile = std::make_shared<VirtualFileHolder>(deviceID.getID(), originalFileID);
    const std::string payload = "file measurement payload\n";
    REQUIRE(virtualFile->openFile());
    (*virtualFile) << payload;
    virtualFile->closeFile();

    REQUIRE(synchedEvents.front()->attachFile(virtualFile));

    const auto& measurements = synchedEvents.front()->getMeasurements();
    REQUIRE(measurements.size() == 1);
    measurements.front()->setMeasurementResult(virtualFile->getID());

    TempDir tempDir("measurement-file-transfer-");
    auto shotRoot = tempDir.path / "shot";
    auto dataPath = shotRoot / "data";
    auto experimentPath = shotRoot / "experiments";

    ResultsPaths paths;
    paths.dataPath = dataPath.string();

    auto collectorFactory = std::make_shared<LocalFileHolderFactory>("collector");
    LocalResultsCollector collector(makeShotID(), paths, collectorFactory);

    std::shared_ptr<VirtualFileServer> sourceFileServer;
    REQUIRE(measurements.front()->getFileServer(sourceFileServer));
    REQUIRE(sourceFileServer != nullptr);

    REQUIRE(collector.addMeasurements(deviceID, measurements, sourceFileServer));
    CHECK_FALSE(sourceFileServer->findFile(originalFileID));

    auto collectedMeasurements = collector.getMeasurements();
    REQUIRE(collectedMeasurements != nullptr);
    auto deviceMeasurements = collectedMeasurements->find(deviceID);
    REQUIRE(deviceMeasurements != collectedMeasurements->end());
    REQUIRE(deviceMeasurements->second.size() == 1);

    auto transferredFileID = deviceMeasurements->second.front()->data().getFileID();
    CHECK(transferredFileID.filename == "measurement.txt");
    CHECK(std::filesystem::path(transferredFileID.path) == dataPath);
    CHECK(readFileToString(std::filesystem::path(transferredFileID.getFullFilename())) == payload);

    tinyxml2::XMLDocument doc;
    auto root = doc.NewElement("measurement");
    doc.InsertEndChild(root);

    std::filesystem::create_directories(experimentPath);
    auto shotFilename = (experimentPath / "shot.xml").string();
    LegacyExperimentXMLBuilder::addValue(root, deviceMeasurements->second.front()->data(), shotFilename);

    auto fileElement = root->FirstChildElement("file");
    REQUIRE(fileElement != nullptr);
    auto filenameElement = fileElement->FirstChildElement("filename");
    REQUIRE(filenameElement != nullptr);
    REQUIRE(filenameElement->GetText() != nullptr);

    auto expectedXmlFilename = (std::filesystem::path("..") / "data" / "measurement.txt").string();
    CHECK(std::string(filenameElement->GetText()) == expectedXmlFilename);
}

TEST_CASE("Measurement BinaryData results are written as raw binary files and documented in legacy XML", "[measurement][binary]")
{
    const DeviceID deviceID("MeasurementDevice", "127.0.0.1", 1);
    auto measurement = std::make_shared<Measurement>(10.0, 7, deviceID, STI::Utils::GraphPathLabel{}, "root");

    const std::vector<unsigned char> payload{0x00, 0x01, 0x7F, 0x80, 0xFF, 0x42, 0x00, 0x33};
    auto* buffer = new unsigned char[payload.size()];
    std::copy(payload.begin(), payload.end(), buffer);

    auto binaryData = std::make_shared<BinaryData>();
    binaryData->assign(buffer, payload.size());
    measurement->setMeasurementResult(binaryData);

    MeasurementVector measurements;
    measurements.push_back(measurement);

    TempDir tempDir("measurement-binary-transfer-");
    auto shotRoot = tempDir.path / "shot";
    auto dataPath = shotRoot / "data";
    auto experimentPath = shotRoot / "experiments";

    ResultsPaths paths;
    paths.dataPath = dataPath.string();

    auto collectorFactory = std::make_shared<LocalFileHolderFactory>("collector");
    LocalResultsCollector collector(makeShotID(), paths, collectorFactory);

    std::shared_ptr<FileServer> sourceFileServer;
    REQUIRE(collector.addMeasurements(deviceID, measurements, sourceFileServer));

    auto collectedMeasurements = collector.getMeasurements();
    REQUIRE(collectedMeasurements != nullptr);
    auto deviceMeasurements = collectedMeasurements->find(deviceID);
    REQUIRE(deviceMeasurements != collectedMeasurements->end());
    REQUIRE(deviceMeasurements->second.size() == 1);

    const auto& collectedData = deviceMeasurements->second.front()->data();
    REQUIRE(collectedData.getType() == MixedValueType::File);

    auto transferredFileID = collectedData.getFileID();
    CHECK(std::filesystem::path(transferredFileID.filename).extension() == ".bin");
    CHECK(std::filesystem::path(transferredFileID.path) == dataPath);

    std::string expectedPayload(reinterpret_cast<const char*>(payload.data()), payload.size());
    CHECK(readFileToString(std::filesystem::path(transferredFileID.getFullFilename())) == expectedPayload);

    tinyxml2::XMLDocument doc;
    auto root = doc.NewElement("measurement");
    doc.InsertEndChild(root);

    std::filesystem::create_directories(experimentPath);
    auto shotFilename = (experimentPath / "shot.xml").string();
    LegacyExperimentXMLBuilder::addValue(root, collectedData, shotFilename);

    auto fileElement = root->FirstChildElement("file");
    REQUIRE(fileElement != nullptr);
    auto filenameElement = fileElement->FirstChildElement("filename");
    REQUIRE(filenameElement != nullptr);
    REQUIRE(filenameElement->GetText() != nullptr);

    auto expectedXmlFilename = (std::filesystem::path("..") / "data" / transferredFileID.filename).string();
    CHECK(std::string(filenameElement->GetText()) == expectedXmlFilename);
}

TEST_CASE("Measurement lazy BinaryData results are pulled into server-local files", "[measurement][binary][lazy]")
{
    const DeviceID deviceID("MeasurementDevice", "127.0.0.1", 1);
    auto measurement = std::make_shared<Measurement>(10.0, 7, deviceID, STI::Utils::GraphPathLabel{}, "root");

    const std::string payload = "lazy-binary-measurement";
    auto stream = std::make_shared<PayloadStream>(payload);
    auto binaryData = std::make_shared<BinaryData>();
    binaryData->attachStream(stream, payload.size(), 1);
    measurement->setMeasurementResult(binaryData);

    MeasurementVector measurements{measurement};

    TempDir tempDir("measurement-lazy-binary-transfer-");
    auto dataPath = tempDir.path / "shot" / "data";

    ResultsPaths paths;
    paths.dataPath = dataPath.string();

    auto collectorFactory = std::make_shared<LocalFileHolderFactory>("collector");
    LocalResultsCollector collector(makeShotID(), paths, collectorFactory);

    std::shared_ptr<FileServer> sourceFileServer;
    REQUIRE(collector.addMeasurements(deviceID, measurements, sourceFileServer));
    CHECK(stream->transferCount == 1);

    auto collectedMeasurements = collector.getMeasurements();
    REQUIRE(collectedMeasurements != nullptr);
    auto deviceMeasurements = collectedMeasurements->find(deviceID);
    REQUIRE(deviceMeasurements != collectedMeasurements->end());
    REQUIRE(deviceMeasurements->second.size() == 1);

    const auto& collectedData = deviceMeasurements->second.front()->data();
    REQUIRE(collectedData.getType() == MixedValueType::File);

    auto transferredFileID = collectedData.getFileID();
    CHECK(std::filesystem::path(transferredFileID.path) == dataPath);
    CHECK(readFileToString(std::filesystem::path(transferredFileID.getFullFilename())) == payload);
}

TEST_CASE("Measurement lazy binary-backed Image results are pulled into server-local image files", "[measurement][image][lazy]")
{
    const DeviceID deviceID("MeasurementDevice", "127.0.0.1", 1);
    auto measurement = std::make_shared<Measurement>(10.0, 7, deviceID, STI::Utils::GraphPathLabel{}, "root");

    const std::string payload = "lazy-image-measurement";
    auto stream = std::make_shared<PayloadStream>(payload);
    auto binaryData = std::make_shared<BinaryData>();
    binaryData->attachStream(stream, payload.size(), 1);

    TempDir tempDir("measurement-lazy-image-transfer-");
    auto image = std::make_shared<Image>("device-origin", (tempDir.path / "source" / "frame.raw").string());
    image->setHeight(12).setWidth(34);
    image->setImageData(binaryData);
    measurement->setMeasurementResult(image);

    MeasurementVector measurements{measurement};

    auto dataPath = tempDir.path / "shot" / "data";

    ResultsPaths paths;
    paths.dataPath = dataPath.string();

    auto collectorFactory = std::make_shared<LocalFileHolderFactory>("collector");
    LocalResultsCollector collector(makeShotID(), paths, collectorFactory);

    auto sourceFileServer = std::make_shared<DummyFileServer>();
    REQUIRE(collector.addMeasurements(deviceID, measurements, sourceFileServer));
    CHECK(stream->transferCount == 1);

    auto collectedMeasurements = collector.getMeasurements();
    REQUIRE(collectedMeasurements != nullptr);
    auto deviceMeasurements = collectedMeasurements->find(deviceID);
    REQUIRE(deviceMeasurements != collectedMeasurements->end());
    REQUIRE(deviceMeasurements->second.size() == 1);

    const auto& collectedData = deviceMeasurements->second.front()->data();
    REQUIRE(collectedData.getType() == MixedValueType::Image);

    auto collectedImage = collectedData.getImage();
    REQUIRE(collectedImage != nullptr);

    std::shared_ptr<BinaryData> cachedData;
    CHECK_FALSE(collectedImage->getData(cachedData));

    std::shared_ptr<FileHolder> cachedFile;
    REQUIRE(collectedImage->getFile(cachedFile));
    CHECK(std::filesystem::path(cachedFile->getID().path) == dataPath);
    CHECK(readFileToString(cachedFile->getFilename()) == payload);
}
