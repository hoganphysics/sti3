#include <catch2/catch_test_macros.hpp>

#include <sti/LocalDevice.h>
#include <sti/device/DeviceCollection.h>
#include <sti/device/DeviceID.h>
#include <sti/engine/EngineJobID.h>
#include <sti/engine/EventEngineJob.h>
#include <sti/engine/EventEngineJobList.h>
#include <sti/engine/ParseJobStatus.h>
#include <sti/engine/ParseResult.h>
#include <sti/engine/PlayJobStatus.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/RawEventGroup.h>
#include <sti/engine/RawEventTarget.h>
#include <sti/engine/RawEventTargetChannel.h>
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
#include <vector>

using STI::Device::ChannelType;
using STI::Device::Device;
using STI::Device::DeviceCollection;
using STI::Device::DeviceID;
using STI::Device::LocalDevice;
using STI::Engine::EngineJobID;
using STI::Engine::EngineJobStatus;
using STI::Engine::EngineParsingMessage;
using STI::Engine::EnginePlayingMessage;
using STI::Engine::EventEngineJob;
using STI::Engine::EventEngineJobList;
using STI::Engine::EventEngineJobType;
using STI::Engine::LocalEventEngineScheduler;
using STI::Engine::ParseID;
using STI::Engine::ParseResult;
using STI::Engine::PlayingMessageType;
using STI::Engine::RawEvent;
using STI::Engine::RawEventGroup;
using STI::Engine::RawEventMap;
using STI::Engine::RawEventTarget;
using STI::Engine::RawEventTargetChannel;
using STI::Engine::RawEventType;
using STI::Engine::ShotConfig;
using STI::Engine::ShotID;
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
        / ("sti3_missing_partner_" + testName + "_" + std::to_string(counter++));

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

class PartnerGeneratingDevice : public LocalDevice
{
public:
    PartnerGeneratingDevice(const std::string& name, unsigned short module, const std::string& targetServer)
        : LocalDevice(name, "127.0.0.1", module, targetServer, makeDeviceConfig(name))
    {
        addChannel(0, ChannelType::Output, MixedValueType::Empty, MixedValueType::Double, "out");
    }

    void setPartnerTarget(const DeviceID& id)
    {
        partnerID = id;
        addEventTarget(id);
    }

    void parseEvents(const RawEventMap& eventsIn, SynchronousEventVector& synchedEvents) override
    {
        RawEventTargetChannel partnerChannel(0);

        for (auto& tuple : eventsIn) {
            synchedEvents.push_back(std::make_shared<CountingEvent>(tuple.first, loadCount, playCount));

            for (auto& rawEvent : tuple.second) {
                partner(partnerID).addEvent(tuple.first, partnerChannel, rawEvent.value(), rawEvent);
            }
        }
    }

    DeviceID partnerID;
    int loadCount = 0;
    int playCount = 0;
};

std::shared_ptr<LocalEventEngineScheduler> schedulerFor(PartnerGeneratingDevice& device)
{
    std::shared_ptr<LocalEventEngineScheduler> scheduler;
    REQUIRE(device.getEngineScheduler(scheduler));
    REQUIRE(scheduler != nullptr);
    return scheduler;
}

std::shared_ptr<STI::Engine::Shot> makeShot(LocalEventEngineScheduler& scheduler, const DeviceID& targetID)
{
    ShotConfig config;
    config.jobSourceID.user = "missing-partner-test";
    config.jobSourceID.machine = "localhost";

    auto group = std::make_shared<RawEventGroup>("root", "");
    group->addEvent(RawEventTarget(targetID, 0), 10.0, MixedValue(1.0), RawEventType::Play);

    return scheduler.createShot(config, group);
}

EngineJobStatus waitForParseTerminal(LocalEventEngineScheduler& scheduler, const ParseID& pid)
{
    for (int i = 0; i < 200; ++i) {
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
    for (int i = 0; i < 200; ++i) {
        auto status = scheduler.getStatus(sid);
        if (status == EngineJobStatus::Completed || status == EngineJobStatus::Canceled) {
            return status;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return scheduler.getStatus(sid);
}

std::size_t countParsingMessages(const std::vector<EngineParsingMessage>& messages, const std::string& name)
{
    std::size_t count = 0;
    for (auto& message : messages) {
        if (message.getName() == name) {
            ++count;
        }
    }
    return count;
}

bool hasPlayError(const std::vector<EnginePlayingMessage>& messages, const std::string& name)
{
    for (auto& message : messages) {
        if (message.getType() == PlayingMessageType::Error && message.getName() == name) {
            return true;
        }
    }
    return false;
}

std::shared_ptr<EventEngineJob> findCompletedPlayJob(LocalEventEngineScheduler& scheduler, const ShotID& sid)
{
    auto jobs = scheduler.getJobs(EventEngineJobList::Completed);
    for (auto& job : jobs) {
        if (job != nullptr && job->getJobID().type == EventEngineJobType::Play && job->getJobID().sid == sid) {
            return job;
        }
    }
    return {};
}

void requireAbstractParse(LocalEventEngineScheduler& scheduler, const ParseID& pid, const DeviceID& missingID)
{
    std::shared_ptr<ParseResult> parseResult;
    REQUIRE(scheduler.getParseResult(pid, parseResult));
    REQUIRE(parseResult != nullptr);

    CHECK(countParsingMessages(parseResult->messages, "Abstract Shot") == 1);

	std::shared_ptr<EventEngineJob> parseJob;
	REQUIRE(scheduler.getJob(EngineJobID(pid), parseJob));
	REQUIRE(parseJob != nullptr);
	CHECK(countParsingMessages(parseJob->getParsingMessages(), "Abstract Shot") == 1);
	CHECK(parseJob->getMissingTargetIDs().contains(missingID));
}

} // namespace

TEST_CASE("Declared missing partner keeps its target ID without entering the device collection")
{
    PartnerGeneratingDevice server("Server", 1, "root");
    DeviceID missingPartner("MissingPartner", "127.0.0.1", 2, server.getID().getID());

    server.setPartnerTarget(missingPartner);

    CHECK(server.partner(missingPartner).getID() == missingPartner);

    std::shared_ptr<DeviceCollection> collection;
    server.getCollection(collection);
    REQUIRE(collection != nullptr);

    std::set<DeviceID> ids;
    collection->getIDs(ids);
    CHECK(ids.empty());
}

TEST_CASE("Missing partner-generated target makes parse abstract and blocks play")
{
    PartnerGeneratingDevice server("Server", 3, "root");
    DeviceID missingPartner("MissingPartner", "127.0.0.1", 4, server.getID().getID());
    server.setPartnerTarget(missingPartner);

    auto scheduler = schedulerFor(server);
    auto shot = makeShot(*scheduler, server.getID());

    auto parseStatus = scheduler->parse(shot);
    REQUIRE(waitForParseTerminal(*scheduler, parseStatus.pid) == EngineJobStatus::Completed);
    requireAbstractParse(*scheduler, parseStatus.pid, missingPartner);

    auto playStatus = scheduler->play(parseStatus.pid, shot->getShotConfig().jobSourceID);
    REQUIRE(waitForShotTerminal(*scheduler, playStatus.sid) == EngineJobStatus::Canceled);

    auto playJob = findCompletedPlayJob(*scheduler, playStatus.sid);
    REQUIRE(playJob != nullptr);
    CHECK(hasPlayError(playJob->getPlayMessages(), "Cannot Play Abstract Shot"));
    CHECK(server.loadCount == 0);
    CHECK(server.playCount == 0);

    std::shared_ptr<STI::Engine::ShotResult> shotResult;
    CHECK_FALSE(scheduler->getShotResult(playStatus.sid, shotResult));
}

TEST_CASE("Missing partner remains abstract when its target server differs from the parsing server")
{
    PartnerGeneratingDevice server("Server", 5, "root");
    DeviceID missingPartner("MissingPartner", "127.0.0.1", 6, "127.0.0.1/99/OtherServer");
    server.setPartnerTarget(missingPartner);

    auto scheduler = schedulerFor(server);
    auto shot = makeShot(*scheduler, server.getID());

    auto parseStatus = scheduler->parse(shot);
    REQUIRE(waitForParseTerminal(*scheduler, parseStatus.pid) == EngineJobStatus::Completed);
    requireAbstractParse(*scheduler, parseStatus.pid, missingPartner);

    auto playStatus = scheduler->play(parseStatus.pid, shot->getShotConfig().jobSourceID);
    REQUIRE(waitForShotTerminal(*scheduler, playStatus.sid) == EngineJobStatus::Canceled);

    auto playJob = findCompletedPlayJob(*scheduler, playStatus.sid);
    REQUIRE(playJob != nullptr);
    CHECK(hasPlayError(playJob->getPlayMessages(), "Cannot Play Abstract Shot"));
    CHECK(server.playCount == 0);
}
