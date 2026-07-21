#include <catch2/catch_test_macros.hpp>

#include <sti/device/DeviceID.h>
#include <sti/engine/EnginePlayingMessage.h>
#include <sti/engine/Measurement.h>
#include <sti/engine/ResultsCollector.h>
#include <sti/utils/Configuration.h>
#include <sti/utils/FileHolder.h>
#include <sti/utils/FileID.h>
#include <sti/utils/FileServer.h>
#include <sti/utils/MixedValue.h>

#include "CompositeFileServer.h"
#include "ORBManager.h"
#include "RemoteResultsCollector.h"
#include "ServantHolder.h"
#include "TResultsCollector_i.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace
{

class FindingFileServer : public STI::Utils::FileServer
{
public:
    explicit FindingFileServer(const STI::Utils::FileID& availableFile)
        : availableFile(availableFile)
    {
    }

    bool addFile(const std::shared_ptr<STI::Utils::FileHolder>&) override { return false; }
    bool findFile(const STI::Utils::FileID& fileID) override { return fileID == availableFile; }
    int getFileSize(const STI::Utils::FileID& fileID) override { return findFile(fileID) ? 1 : 0; }
    bool transferFile(const STI::Utils::FileID&, const std::shared_ptr<STI::Utils::FileHolder>&,
        STI::Utils::FileTransferType) override { return false; }
    bool transferFilePartial(const STI::Utils::FileID&, const std::shared_ptr<STI::Utils::FileHolder>&,
        int, int) override { return false; }
    bool deleteFile(const STI::Utils::FileID&) override { return false; }

private:
    STI::Utils::FileID availableFile;
};

class CapturingResultsCollector : public STI::Engine::ResultsCollector
{
public:
    explicit CapturingResultsCollector(const STI::Utils::FileID& expectedFile)
        : expectedFile(expectedFile)
    {
    }

    STI::Engine::ShotID getShotID() const override { return {}; }

    bool addMeasurements(const STI::Device::DeviceID&,
        const STI::Engine::MeasurementVector& measurements,
        const std::shared_ptr<STI::Utils::FileServer>& sourceFileServer) override
    {
        measurementCount = measurements.size();
        sourceFoundFile = sourceFileServer != nullptr && sourceFileServer->findFile(expectedFile);
        return sourceFoundFile;
    }

    bool addAttributes(const STI::Device::DeviceID&,
        const std::map<std::string, std::string>&) override { return true; }
    bool addVersionInfo(const STI::Device::DeviceID&,
        const std::vector<STI::Device::VersionInfo>&) override { return true; }
    bool addMessages(const std::vector<STI::Engine::EnginePlayingMessage>&) override { return true; }

    std::size_t measurementCount = 0;
    bool sourceFoundFile = false;

private:
    STI::Utils::FileID expectedFile;
};

} // namespace

TEST_CASE("RemoteResultsCollector exports generic composite file servers", "[resultscollector][fileserver][network]")
{
    auto orbManager = STI::Network::ORBManager::getInstance(STI::Utils::Configuration(), "sti3_test_network");
    REQUIRE(orbManager != nullptr);
    REQUIRE(orbManager->isPOAactive());

    STI::Utils::FileID expectedFile;
    expectedFile.origin = "source-device";
    expectedFile.persistenceLocation = "source-device";
    expectedFile.path = "measurements";
    expectedFile.filename = "result.bin";

    STI::Utils::FileID otherFile = expectedFile;
    otherFile.filename = "other.bin";

    auto firstSource = std::make_shared<FindingFileServer>(otherFile);
    auto secondSource = std::make_shared<FindingFileServer>(expectedFile);
    auto compositeSource = std::make_shared<STI::Device::CompositeFileServer>(
        std::vector<std::shared_ptr<STI::Utils::FileServer>>{ firstSource, secondSource });

    CapturingResultsCollector localCollector(expectedFile);
    STI::Network::ServantHolder<STI::TNetwork::TResultsCollector_i, STI::TNetwork::TResultsCollector>
        collectorServant(new STI::TNetwork::TResultsCollector_i(&localCollector));
    STI::Network::RemoteResultsCollector remoteCollector(collectorServant.getRefVar());

    const STI::Device::DeviceID deviceID("MeasurementDevice", "127.0.0.1", 1);
    auto measurement = std::make_shared<STI::Engine::Measurement>(
        10.0, 3, deviceID, STI::Utils::GraphPathLabel{}, "root");
    measurement->setMeasurementResult(STI::Utils::MixedValue(4.5));

    REQUIRE(remoteCollector.addMeasurements(deviceID, { measurement }, compositeSource));
    CHECK(localCollector.measurementCount == 1);
    CHECK(localCollector.sourceFoundFile);
}
