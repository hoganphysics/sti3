#include <catch2/catch_test_macros.hpp>

#include <sti/LocalDevice.h>
#include <sti/device/DeviceID.h>
#include <sti/engine/AddSequenceStatus.h>
#include <sti/engine/EngineJobSourceID.h>
#include <sti/engine/EngineJobStatus.h>
#include <sti/engine/ParsedVar.h>
#include <sti/engine/ParseJobStatus.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/RawEventGroup.h>
#include <sti/engine/RawEventTarget.h>
#include <sti/engine/Sequence.h>
#include <sti/engine/SequenceID.h>
#include <sti/engine/Shot.h>
#include <sti/engine/ShotConfig.h>
#include <sti/engine/SynchronousEvent.h>
#include <sti/utils/Configuration.h>
#include <sti/utils/MixedValue.h>

#include "LocalEventEngineScheduler.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <memory>
#include <set>
#include <string>
#include <thread>

using STI::Device::ChannelType;
using STI::Device::DeviceID;
using STI::Device::LocalDevice;
using STI::Engine::AddSequenceStatus;
using STI::Engine::EngineJobSourceID;
using STI::Engine::EngineJobStatus;
using STI::Engine::LocalEventEngineScheduler;
using STI::Engine::ParsedVar;
using STI::Engine::ParseID;
using STI::Engine::RawEventGroup;
using STI::Engine::RawEventMap;
using STI::Engine::RawEventTarget;
using STI::Engine::RawEventType;
using STI::Engine::Sequence;
using STI::Engine::SequenceEntryID;
using STI::Engine::SequenceIndex;
using STI::Engine::SequenceSchedulingMode;
using STI::Engine::SequenceType;
using STI::Engine::ShotConfig;
using STI::Engine::ShotType;
using STI::Engine::SynchronousEvent;
using STI::Engine::SynchronousEventVector;
using STI::Utils::Configuration;
using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;

namespace {

Configuration makeDeviceConfig(const std::string& testName, const std::string& sequenceMode)
{
    static std::atomic<unsigned> counter{0};

    auto root = std::filesystem::temp_directory_path()
        / ("sti3_sequence_mode_" + testName + "_" + std::to_string(counter++));

    Configuration config;
    config.set("PersistenceManager", "root path", root.string());
    config.set("PersistenceManager", "device subdirectory", "device");
    config.set("EngineManager", "Engine Count", 1);
    config.set("EventScheduler", "Sequence Mode", sequenceMode);
    return config;
}

class NoopEvent : public SynchronousEvent
{
public:
    explicit NoopEvent(double time) : SynchronousEvent(time) {}

    void loadEvent() override {}
    void playEvent() override {}
    void collectMeasurementData() override {}
    void stopEvent() override {}
    void pauseEvent() override {}
    void unpauseEvent(bool) override {}
};

class SequenceModeDevice : public LocalDevice
{
public:
    SequenceModeDevice(const std::string& name, unsigned short module, const std::string& sequenceMode)
        : LocalDevice(name, "127.0.0.1", module, "root", makeDeviceConfig(name, sequenceMode))
    {
        addChannel(0, ChannelType::Output, MixedValueType::Empty, MixedValueType::Double, "out");
    }

    void parseEvents(const RawEventMap& eventsIn, SynchronousEventVector& synchedEvents) override
    {
        for (auto& tuple : eventsIn) {
            synchedEvents.push_back(std::make_shared<NoopEvent>(tuple.first));
        }
    }
};

std::shared_ptr<LocalEventEngineScheduler> schedulerFor(SequenceModeDevice& device)
{
    std::shared_ptr<LocalEventEngineScheduler> scheduler;
    REQUIRE(device.getEngineScheduler(scheduler));
    REQUIRE(scheduler != nullptr);
    return scheduler;
}

EngineJobSourceID makeSource()
{
    EngineJobSourceID source;
    source.user = "sequence-mode-test";
    source.machine = "localhost";
    return source;
}

std::shared_ptr<Sequence> makeSequence(const EngineJobSourceID& source)
{
    auto sequence = std::make_shared<Sequence>(SequenceType::Closed);
    sequence->shotConfig.jobSourceID = source;
    sequence->shotConfig.shotType = ShotType::Sequence;
    sequence->addEntry(SequenceIndex(0, 0), std::set<ParsedVar>());
    return sequence;
}

std::shared_ptr<STI::Engine::Shot> makeSequenceShot(LocalEventEngineScheduler& scheduler,
                                                    const DeviceID& targetID,
                                                    const EngineJobSourceID& source)
{
    ShotConfig config;
    config.jobSourceID = source;
    config.shotType = ShotType::SequenceEntry;

    auto group = std::make_shared<RawEventGroup>("root", "");
    group->addEvent(RawEventTarget(targetID, 0), 1'000'000.0, MixedValue(1.0), RawEventType::Play);

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

SequenceEntryID firstEntry(const STI::Engine::SequenceID& seqid)
{
    return SequenceEntryID(seqid, SequenceIndex(0, 0));
}

} // namespace

TEST_CASE("SequenceSchedulingMode parses config values", "[eventengine][sequence]")
{
    SequenceSchedulingMode mode = SequenceSchedulingMode::Normal;
    REQUIRE(LocalEventEngineScheduler::parseSequenceSchedulingMode("interleaved", mode));
    CHECK(mode == SequenceSchedulingMode::Interleaved);

    REQUIRE(LocalEventEngineScheduler::parseSequenceSchedulingMode("Normal", mode));
    CHECK(mode == SequenceSchedulingMode::Normal);

    CHECK_FALSE(LocalEventEngineScheduler::parseSequenceSchedulingMode("unknown", mode));
}

TEST_CASE("normal sequence scheduling uses first submitted sequence job for priority", "[eventengine][sequence]")
{
    SequenceModeDevice device("NormalSequenceModeDevice", 31, "Normal");
    auto scheduler = schedulerFor(device);
    auto source = makeSource();

    auto idleSequence = makeSequence(source);
    auto activeSequence = makeSequence(source);

    auto idleStatus = scheduler->addSequence(idleSequence, source);
    REQUIRE(idleStatus.status == EngineJobStatus::New);

    auto activeStatus = scheduler->addSequence(activeSequence, source);
    REQUIRE(activeStatus.status == EngineJobStatus::New);

    auto shot = makeSequenceShot(*scheduler, device.getID(), source);
    auto parseStatus = scheduler->parse(shot, firstEntry(activeStatus.seqid));

    CHECK(waitForParseTerminal(*scheduler, parseStatus.pid) == EngineJobStatus::Completed);
    CHECK(scheduler->getStatus(idleStatus.seqid) == EngineJobStatus::New);
}

TEST_CASE("interleaved sequence scheduling does not block on another active sequence", "[eventengine][sequence]")
{
    SequenceModeDevice device("InterleavedSequenceModeDevice", 32, "Interleaved");
    auto scheduler = schedulerFor(device);
    auto source = makeSource();

    auto firstSequence = makeSequence(source);
    auto secondSequence = makeSequence(source);

    auto firstStatus = scheduler->addSequence(firstSequence, source);
    REQUIRE(firstStatus.status == EngineJobStatus::New);

    auto secondStatus = scheduler->addSequence(secondSequence, source);
    REQUIRE(secondStatus.status == EngineJobStatus::New);

    auto firstShot = makeSequenceShot(*scheduler, device.getID(), source);
    auto firstParse = scheduler->parse(firstShot, firstEntry(firstStatus.seqid));
    REQUIRE(waitForParseTerminal(*scheduler, firstParse.pid) == EngineJobStatus::Completed);

    auto secondShot = makeSequenceShot(*scheduler, device.getID(), source);
    auto secondParse = scheduler->parse(secondShot, firstEntry(secondStatus.seqid));
    CHECK(waitForParseTerminal(*scheduler, secondParse.pid) == EngineJobStatus::Completed);
}
