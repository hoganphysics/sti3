#include <catch2/catch_test_macros.hpp>

#include <sti/LocalDevice.h>
#include <sti/device/DeviceID.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/SynchronousEvent.h>
#include <sti/utils/Configuration.h>
#include <sti/utils/MixedValue.h>

#include "LocalFileServer.h"
#include "TransientRepository.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <memory>
#include <string>
#include <system_error>

using STI::Device::ChannelType;
using STI::Device::DeviceID;
using STI::Device::LocalDevice;
using STI::Engine::ParseID;
using STI::Engine::RawEvent;
using STI::Engine::RawEventMap;
using STI::Engine::ShotConfig;
using STI::Engine::ShotID;
using STI::Engine::ShotType;
using STI::Engine::SynchronousEvent;
using STI::Engine::SynchronousEventVector;
using STI::Engine::TransientRepository;
using STI::Utils::Configuration;
using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;

namespace {

std::filesystem::path makeTempRoot(const std::string& testName)
{
    static std::atomic<unsigned> counter{0};

    auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    auto root = std::filesystem::temp_directory_path()
        / ("sti3_localdevice_write_" + testName + "_" + std::to_string(now) + "_" + std::to_string(counter++));

    std::error_code ec;
    std::filesystem::remove_all(root, ec);
    return root;
}

Configuration makeDeviceConfig(const std::filesystem::path& root)
{
    Configuration config;
    config.set("PersistenceManager", "root path", root.string());
    config.set("PersistenceManager", "device subdirectory", "device");
    config.set("EngineManager", "Engine Count", 1);
    return config;
}

class CountingEvent : public SynchronousEvent
{
public:
    CountingEvent(double time, int& loadCount, int& playCount)
        : SynchronousEvent(time), loadCount(loadCount), playCount(playCount) {}

    void loadEvent() override { ++loadCount; }
    void playEvent() override { ++playCount; }
    void collectMeasurementData() override {}
    void stopEvent() override {}
    void pauseEvent() override {}
    void unpauseEvent(bool) override {}

private:
    int& loadCount;
    int& playCount;
};

class AsyncWriteDevice : public LocalDevice
{
public:
    explicit AsyncWriteDevice(const std::filesystem::path& root)
        : LocalDevice("AsyncWriteDevice", "127.0.0.1", 1, "root", makeDeviceConfig(root))
    {
        addOutputChannel(0, MixedValueType::Double, "out");
    }

    double getMinimumEventStartTime() override { return 0.0; }

    void parseEvents(const RawEventMap& events, SynchronousEventVector& synchedEvents) override
    {
        for (auto& tuple : events) {
            synchedEvents.push_back(std::make_shared<CountingEvent>(tuple.first, loadCount, playCount));
        }
    }

    int loadCount = 0;
    int playCount = 0;
};

class FixedMeasurementEvent : public SynchronousEvent
{
public:
    FixedMeasurementEvent(double time, const std::vector<RawEvent>& events, MixedValue result)
        : SynchronousEvent(time), result(result)
    {
        for (const auto& event : events) {
            addMeasurement(event);
        }
    }

    void loadEvent() override {}
    void playEvent() override {}
    void collectMeasurementData() override
    {
        for (unsigned i = 0; i < getMeasurements().size(); ++i) {
            setMeasurementResult(i, result);
        }
    }
    void stopEvent() override {}
    void pauseEvent() override {}
    void unpauseEvent(bool) override {}

private:
    MixedValue result;
};

class MeasurementStateDevice : public LocalDevice
{
public:
    MeasurementStateDevice(const std::filesystem::path& root,
                           MixedValueType inputType,
                           MixedValueType outputType,
                           MixedValue result)
        : LocalDevice("MeasurementStateDevice", "127.0.0.1", 1, "root", makeDeviceConfig(root)),
          result(result)
    {
        addInputChannel(0, inputType, outputType, "in");
    }

    double getMinimumEventStartTime() override { return 0.0; }

    void parseEvents(const RawEventMap& events, SynchronousEventVector& synchedEvents) override
    {
        for (const auto& tuple : events) {
            synchedEvents.push_back(std::make_shared<FixedMeasurementEvent>(tuple.first, tuple.second, result));
        }
    }

private:
    MixedValue result;
};

ShotID makeSingleUndocumentedShotID()
{
    ShotConfig config;
    config.shotType = ShotType::SingleUndocumented;
    config.jobSourceID.user = "localdevice-write-test";
    config.jobSourceID.machine = "localhost";

    auto pid = ParseID::generateUniqueID(config.jobSourceID);
    pid.shotType = config.shotType;
    return ShotID::generateUniqueID(pid, config.jobSourceID);
}

} // namespace

TEST_CASE("TransientRepository prepares paths lazily for undocumented shots", "[localdevice][write][transient]")
{
    auto root = makeTempRoot("lazy_transient");
    auto transientRoot = root / "transient_cache";

    auto server = std::make_shared<STI::Utils::LocalFileServer>(DeviceID("TransientDevice", "127.0.0.1", 1));
    TransientRepository repository(root.string(), server);

    auto sid = makeSingleUndocumentedShotID();
    auto paths = repository.preparePaths(sid);

    CHECK(paths.basePath == (transientRoot / "tmp").string());
    CHECK_FALSE(std::filesystem::exists(transientRoot));
}

TEST_CASE("LocalDevice write handles rapid async SingleUndocumented shots without transient directories", "[localdevice][write][transient]")
{
    auto root = makeTempRoot("rapid_async");
    AsyncWriteDevice device(root);

    constexpr int WriteCount = 50;
    for (int i = 0; i < WriteCount; ++i) {
        REQUIRE(device.write(0, MixedValue(static_cast<double>(i))));
    }

    CHECK(device.loadCount == WriteCount);
    CHECK(device.playCount == WriteCount);
    CHECK_FALSE(std::filesystem::exists(root / "device" / "transient_cache"));
}

TEST_CASE("LocalEventEngine updates input channel lastValue and lastMeasurement for measurements", "[localdevice][channel]")
{
    auto root = makeTempRoot("measurement_state");
    MeasurementStateDevice device(root, MixedValueType::Double, MixedValueType::Int, MixedValue(6.5));

    MixedValue data;
    REQUIRE(device.read(0, MixedValue(3), data));
    CHECK(data == MixedValue(6.5));

    std::shared_ptr<STI::Device::ChannelManager> manager;
    device.getChannelManager(manager);
    REQUIRE(manager != nullptr);

    std::shared_ptr<STI::Device::Channel> channel;
    REQUIRE(manager->getChannel(0, channel));
    CHECK(channel->getLastValue() == MixedValue(3));
    CHECK(channel->getLastMeasurement() == MixedValue(6.5));
}

TEST_CASE("LocalEventEngine leaves lastValue empty for measurement channels without output type", "[localdevice][channel]")
{
    auto root = makeTempRoot("measurement_empty_output");
    MeasurementStateDevice device(root, MixedValueType::Double, MixedValueType::Empty, MixedValue(8.5));

    MixedValue data;
    REQUIRE(device.read(0, data));
    CHECK(data == MixedValue(8.5));

    std::shared_ptr<STI::Device::ChannelManager> manager;
    device.getChannelManager(manager);
    REQUIRE(manager != nullptr);

    std::shared_ptr<STI::Device::Channel> channel;
    REQUIRE(manager->getChannel(0, channel));
    CHECK(channel->getLastValue().isEmpty());
    CHECK(channel->getLastMeasurement() == MixedValue(8.5));
}
