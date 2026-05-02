#include <catch2/catch_test_macros.hpp>

#include <sti/LocalDevice.h>
#include <sti/device/DeviceID.h>
#include <sti/engine/EngineID.h>
#include <sti/engine/EngineJobID.h>
#include <sti/engine/EngineJobStatus.h>
#include <sti/engine/EngineState.h>
#include <sti/engine/EventEngineJob.h>
#include <sti/engine/EventEngineJobList.h>
#include <sti/engine/ParseJobStatus.h>
#include <sti/engine/PlayJobStatus.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/RawEventGroup.h>
#include <sti/engine/RawEventTarget.h>
#include <sti/engine/Shot.h>
#include <sti/engine/ShotConfig.h>
#include <sti/engine/ShotResult.h>
#include <sti/engine/SynchronousEvent.h>
#include <sti/utils/Configuration.h>
#include <sti/utils/MixedValue.h>

#include "LocalEventEngineScheduler.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <memory>
#include <string>
#include <thread>

using STI::Device::ChannelType;
using STI::Device::DeviceID;
using STI::Device::LocalDevice;
using STI::Engine::EngineID;
using STI::Engine::EngineJobID;
using STI::Engine::EngineJobStatus;
using STI::Engine::EngineState;
using STI::Engine::EventEngineJobList;
using STI::Engine::LocalEventEngineScheduler;
using STI::Engine::ParseID;
using STI::Engine::RawEventGroup;
using STI::Engine::RawEventMap;
using STI::Engine::RawEventTarget;
using STI::Engine::RawEventType;
using STI::Engine::ShotConfig;
using STI::Engine::ShotID;
using STI::Engine::ShotResult;
using STI::Engine::ShotResultStatus;
using STI::Engine::SynchronousEvent;
using STI::Engine::SynchronousEventVector;
using STI::Utils::Configuration;
using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;

namespace {

Configuration makeDeviceConfig(const std::string& testName)
{
    static std::atomic<unsigned> counter{0};

    auto root = std::filesystem::temp_directory_path()
        / ("sti3_stop_engine_" + testName + "_" + std::to_string(counter++));

    Configuration config;
    config.set("PersistenceManager", "root path", root.string());
    config.set("PersistenceManager", "device subdirectory", "device");
    config.set("EngineManager", "Engine Count", 1);
    return config;
}

class CountingEvent : public SynchronousEvent
{
public:
    CountingEvent(double time, int& loadCount, int& playCount, int& stopCount)
        : SynchronousEvent(time), loadCount(loadCount), playCount(playCount), stopCount(stopCount) {}

    void loadEvent() override { ++loadCount; }
    void playEvent() override { ++playCount; }
    void collectMeasurementData() override {}
    void stopEvent() override { ++stopCount; }
    void pauseEvent() override {}
    void unpauseEvent(bool) override {}

private:
    int& loadCount;
    int& playCount;
    int& stopCount;
};

class TimedEventDevice : public LocalDevice
{
public:
    TimedEventDevice(const std::string& name, unsigned short module)
        : LocalDevice(name, "127.0.0.1", module, "root", makeDeviceConfig(name))
    {
        addChannel(0, ChannelType::Output, MixedValueType::Empty, MixedValueType::Double, "out");
    }

    void parseEvents(const RawEventMap& eventsIn, SynchronousEventVector& synchedEvents) override
    {
        for (auto& tuple : eventsIn) {
            synchedEvents.push_back(std::make_shared<CountingEvent>(tuple.first, loadCount, playCount, stopCount));
        }
    }

    int loadCount = 0;
    int playCount = 0;
    int stopCount = 0;
};

std::shared_ptr<LocalEventEngineScheduler> schedulerFor(TimedEventDevice& device)
{
    std::shared_ptr<LocalEventEngineScheduler> scheduler;
    REQUIRE(device.getEngineScheduler(scheduler));
    REQUIRE(scheduler != nullptr);
    return scheduler;
}

std::shared_ptr<STI::Engine::Shot> makeShot(LocalEventEngineScheduler& scheduler,
                                            const DeviceID& targetID,
                                            double eventTimeNs)
{
    ShotConfig config;
    config.jobSourceID.user = "stop-engine-test";
    config.jobSourceID.machine = "localhost";

    auto group = std::make_shared<RawEventGroup>("root", "");
    group->addEvent(RawEventTarget(targetID, 0), eventTimeNs, MixedValue(1.0), RawEventType::Play);

    return scheduler.createShot(config, group);
}

EngineJobStatus waitForParseTerminal(LocalEventEngineScheduler& scheduler, const ParseID& pid)
{
    for (int i = 0; i < 300; ++i) {
        auto status = scheduler.getStatus(pid);
        if (status == EngineJobStatus::Completed || status == EngineJobStatus::Canceled) {
            return status;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return scheduler.getStatus(pid);
}

EngineJobStatus waitForShotTerminal(LocalEventEngineScheduler& scheduler, const ShotID& sid)
{
    for (int i = 0; i < 300; ++i) {
        auto status = scheduler.getStatus(sid);
        if (status == EngineJobStatus::Completed || status == EngineJobStatus::Canceled) {
            return status;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return scheduler.getStatus(sid);
}

bool waitForEngineState(LocalEventEngineScheduler& scheduler, const EngineID& engineID, EngineState state)
{
    for (int i = 0; i < 300; ++i) {
        if (scheduler.getEngineState(engineID) == state) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return scheduler.getEngineState(engineID) == state;
}

bool waitForShotResultStatus(LocalEventEngineScheduler& scheduler,
                             const ShotID& sid,
                             ShotResultStatus status,
                             std::shared_ptr<ShotResult>& result)
{
    for (int i = 0; i < 300; ++i) {
        if (scheduler.getShotResult(sid, result) && result != nullptr && result->status == status) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return scheduler.getShotResult(sid, result) && result != nullptr && result->status == status;
}

bool runningListContains(LocalEventEngineScheduler& scheduler, const EngineJobID& target)
{
    for (auto& job : scheduler.getJobs(EventEngineJobList::Running)) {
        if (job != nullptr && job->getJobID() == target) {
            return true;
        }
    }
    return false;
}

bool hasPlayMessage(const ShotResult& result, const std::string& name)
{
    for (auto& message : result.messages) {
        if (message.getName() == name) {
            return true;
        }
    }
    return false;
}

} // namespace

TEST_CASE("stopEngine cancels active play job and releases scheduler", "[eventengine][scheduler][stopengine]")
{
    TimedEventDevice device("StopEngineDevice", 21);
    auto scheduler = schedulerFor(device);
    EngineID syncEngineID(1);

    auto shot = makeShot(*scheduler, device.getID(), 200'000'000.0);
    auto parseStatus = scheduler->parse(shot);
    REQUIRE(waitForParseTerminal(*scheduler, parseStatus.pid) == EngineJobStatus::Completed);

    auto playStatus = scheduler->play(parseStatus.pid, shot->getShotConfig().jobSourceID);
    REQUIRE(waitForEngineState(*scheduler, syncEngineID, EngineState::Playing));

    scheduler->stopEngine(syncEngineID);

    REQUIRE(waitForShotTerminal(*scheduler, playStatus.sid) == EngineJobStatus::Canceled);
    CHECK_FALSE(runningListContains(*scheduler, EngineJobID(playStatus.sid)));

    std::shared_ptr<ShotResult> stoppedResult;
    REQUIRE(waitForShotResultStatus(*scheduler, playStatus.sid, ShotResultStatus::CanceledByUser, stoppedResult));
    CHECK(hasPlayMessage(*stoppedResult, "Play canceled"));
    CHECK(device.playCount == 0);
    CHECK(device.stopCount >= 1);

    auto nextShot = makeShot(*scheduler, device.getID(), 1'000.0);
    auto nextParse = scheduler->parse(nextShot);
    REQUIRE(waitForParseTerminal(*scheduler, nextParse.pid) == EngineJobStatus::Completed);

    auto nextPlay = scheduler->play(nextParse.pid, nextShot->getShotConfig().jobSourceID);
    REQUIRE(waitForShotTerminal(*scheduler, nextPlay.sid) == EngineJobStatus::Completed);

    std::shared_ptr<ShotResult> completedResult;
    REQUIRE(waitForShotResultStatus(*scheduler, nextPlay.sid, ShotResultStatus::Success, completedResult));
}
