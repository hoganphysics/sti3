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
#include <sti/utils/Distributer.h>
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
using STI::Utils::Distributer;
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

            if (!partnerID.empty()) {
                for (auto& rawEvent : tuple.second) {
                    if (useDirectRawPartnerEvents) {
                        RawEvent partnerEvent(
                            RawEventTarget(partnerID, partnerChannel),
                            tuple.first,
                            rawEvent.value(),
                            0,
                            rawEvent.type());
                        addEvent(partnerEvent, rawEvent);
                    }
                    else {
                        partner(partnerID).addEvent(tuple.first, partnerChannel, rawEvent.value(), rawEvent);
                    }
                }
            }
        }
    }

    DeviceID partnerID;
    bool useDirectRawPartnerEvents = false;
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

std::unique_ptr<Distributer<DeviceID, Device>> distributeDevices(std::initializer_list<std::shared_ptr<PartnerGeneratingDevice>> devices)
{
    auto distributer = std::make_unique<Distributer<DeviceID, Device>>();
    for (auto& device : devices) {
        REQUIRE(distributer->add(device->getID(), device));
    }
    return distributer;
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

void requireConcreteParse(LocalEventEngineScheduler& scheduler, const ParseID& pid)
{
    std::shared_ptr<ParseResult> parseResult;
    REQUIRE(scheduler.getParseResult(pid, parseResult));
    REQUIRE(parseResult != nullptr);

    CHECK(countParsingMessages(parseResult->messages, "Abstract Shot") == 0);

	std::shared_ptr<EventEngineJob> parseJob;
	REQUIRE(scheduler.getJob(EngineJobID(pid), parseJob));
	REQUIRE(parseJob != nullptr);
	CHECK(countParsingMessages(parseJob->getParsingMessages(), "Abstract Shot") == 0);
	CHECK(parseJob->getMissingTargetIDs().empty());
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

TEST_CASE("Event target declared without target server is owned by local device")
{
    PartnerGeneratingDevice server("Server", 2, "root");
    DeviceID targetWithoutServer("NoServerTarget", "127.0.0.1", 7);

    server.setPartnerTarget(targetWithoutServer);

    std::set<DeviceID> eventTargets;
    server.getEventTargets(eventTargets);
    REQUIRE(eventTargets.size() == 1);

    auto declaredTarget = *eventTargets.begin();
    CHECK(declaredTarget.getID() == targetWithoutServer.getID());
    CHECK(declaredTarget.getTargetServerID() == server.getID().getID());
    CHECK(server.partner(targetWithoutServer).getID().getTargetServerID() == server.getID().getID());
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

TEST_CASE("Connected partner-generated target resolves when target server differs from parsing device")
{
    DeviceID serverID("Server1", "127.0.0.1", 50, "root");
    auto dev1 = std::make_shared<PartnerGeneratingDevice>("Dev1", 51, serverID.getID());
    auto dev2 = std::make_shared<PartnerGeneratingDevice>("Dev2", 52, serverID.getID());
    dev1->setPartnerTarget(dev2->getID());

    auto distributer = distributeDevices({dev1, dev2});

    std::shared_ptr<DeviceCollection> collection;
    dev1->getCollection(collection);
    REQUIRE(collection != nullptr);
    CHECK(collection->contains(dev2->getID()));

    auto scheduler = schedulerFor(*dev1);
    auto shot = makeShot(*scheduler, dev1->getID());

    auto parseStatus = scheduler->parse(shot);
    REQUIRE(waitForParseTerminal(*scheduler, parseStatus.pid) == EngineJobStatus::Completed);
    requireConcreteParse(*scheduler, parseStatus.pid);

    auto playStatus = scheduler->play(parseStatus.pid, shot->getShotConfig().jobSourceID);
    REQUIRE(waitForShotTerminal(*scheduler, playStatus.sid) == EngineJobStatus::Completed);

    CHECK(dev1->loadCount == 1);
    CHECK(dev1->playCount == 1);
    CHECK(dev2->loadCount == 1);
    CHECK(dev2->playCount == 1);
}

TEST_CASE("Server-owned partner-generated target resolves through normal target server")
{
    auto server = std::make_shared<PartnerGeneratingDevice>("UsualServer1", 70, "root");
    auto dev1 = std::make_shared<PartnerGeneratingDevice>("UsualDev1", 71, server->getID().getID());
    auto dev2 = std::make_shared<PartnerGeneratingDevice>("UsualDev2", 72, server->getID().getID());
    dev1->setPartnerTarget(dev2->getID());

    auto distributer = distributeDevices({server, dev1, dev2});

    std::shared_ptr<DeviceCollection> collection;
    server->getCollection(collection);
    REQUIRE(collection != nullptr);
    CHECK(collection->contains(dev1->getID()));
    CHECK(collection->contains(dev2->getID()));

    auto scheduler = schedulerFor(*server);
    auto shot = makeShot(*scheduler, dev1->getID());

    auto parseStatus = scheduler->parse(shot);
    REQUIRE(waitForParseTerminal(*scheduler, parseStatus.pid) == EngineJobStatus::Completed);
    requireConcreteParse(*scheduler, parseStatus.pid);

    auto playStatus = scheduler->play(parseStatus.pid, shot->getShotConfig().jobSourceID);
    REQUIRE(waitForShotTerminal(*scheduler, playStatus.sid) == EngineJobStatus::Completed);

    CHECK(server->loadCount == 0);
    CHECK(server->playCount == 0);
    CHECK(dev1->loadCount == 1);
    CHECK(dev1->playCount == 1);
    CHECK(dev2->loadCount == 1);
    CHECK(dev2->playCount == 1);
}

TEST_CASE("Connected event target can be declared without target server ID")
{
    DeviceID serverID("Server1", "127.0.0.1", 55, "root");
    auto dev1 = std::make_shared<PartnerGeneratingDevice>("NoServerDev1", 56, serverID.getID());
    auto dev2 = std::make_shared<PartnerGeneratingDevice>("NoServerDev2", 57, serverID.getID());
    DeviceID dev2WithoutServer(dev2->getID().getName(), dev2->getID().getAddress(), dev2->getID().getModule());
    dev1->setPartnerTarget(dev2WithoutServer);

    std::set<DeviceID> eventTargets;
    dev1->getEventTargets(eventTargets);
    REQUIRE(eventTargets.size() == 1);
    CHECK(eventTargets.begin()->getTargetServerID() == dev1->getID().getID());

    auto distributer = distributeDevices({dev1, dev2});

    CHECK(dev1->partner(dev2WithoutServer).getID().getTargetServerID() == serverID.getID());

    auto scheduler = schedulerFor(*dev1);
    auto shot = makeShot(*scheduler, dev1->getID());

    auto parseStatus = scheduler->parse(shot);
    REQUIRE(waitForParseTerminal(*scheduler, parseStatus.pid) == EngineJobStatus::Completed);
    requireConcreteParse(*scheduler, parseStatus.pid);

    auto playStatus = scheduler->play(parseStatus.pid, shot->getShotConfig().jobSourceID);
    REQUIRE(waitForShotTerminal(*scheduler, playStatus.sid) == EngineJobStatus::Completed);

    CHECK(dev1->playCount == 1);
    CHECK(dev2->playCount == 1);
}

TEST_CASE("Direct raw partner events resolve without target server ID")
{
    DeviceID serverID("Server1", "127.0.0.1", 58, "root");
    auto dev1 = std::make_shared<PartnerGeneratingDevice>("DirectRawDev1", 59, serverID.getID());
    auto dev2 = std::make_shared<PartnerGeneratingDevice>("DirectRawDev2", 60, serverID.getID());
    DeviceID dev2WithoutServer(dev2->getID().getName(), dev2->getID().getAddress(), dev2->getID().getModule());
    dev1->setPartnerTarget(dev2WithoutServer);
    dev1->useDirectRawPartnerEvents = true;

    auto distributer = distributeDevices({dev1, dev2});

    auto scheduler = schedulerFor(*dev1);
    auto shot = makeShot(*scheduler, dev1->getID());

    auto parseStatus = scheduler->parse(shot);
    REQUIRE(waitForParseTerminal(*scheduler, parseStatus.pid) == EngineJobStatus::Completed);
    requireConcreteParse(*scheduler, parseStatus.pid);

    auto playStatus = scheduler->play(parseStatus.pid, shot->getShotConfig().jobSourceID);
    REQUIRE(waitForShotTerminal(*scheduler, playStatus.sid) == EngineJobStatus::Completed);

    CHECK(dev1->playCount == 1);
    CHECK(dev2->playCount == 1);
}

TEST_CASE("Nested connected partner-generated targets resolve through acting partner servers")
{
    DeviceID serverID("Server1", "127.0.0.1", 60, "root");
    auto dev1 = std::make_shared<PartnerGeneratingDevice>("NestedDev1", 61, serverID.getID());
    auto dev2 = std::make_shared<PartnerGeneratingDevice>("NestedDev2", 62, serverID.getID());
    auto dev3 = std::make_shared<PartnerGeneratingDevice>("NestedDev3", 63, serverID.getID());
    dev1->setPartnerTarget(dev2->getID());
    dev2->setPartnerTarget(dev3->getID());

    auto distributer = distributeDevices({dev1, dev2, dev3});

    auto scheduler = schedulerFor(*dev1);
    auto shot = makeShot(*scheduler, dev1->getID());

    auto parseStatus = scheduler->parse(shot);
    REQUIRE(waitForParseTerminal(*scheduler, parseStatus.pid) == EngineJobStatus::Completed);
    requireConcreteParse(*scheduler, parseStatus.pid);

    auto playStatus = scheduler->play(parseStatus.pid, shot->getShotConfig().jobSourceID);
    REQUIRE(waitForShotTerminal(*scheduler, playStatus.sid) == EngineJobStatus::Completed);

    CHECK(dev1->playCount == 1);
    CHECK(dev2->playCount == 1);
    CHECK(dev3->playCount == 1);
}
