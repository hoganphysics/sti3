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
