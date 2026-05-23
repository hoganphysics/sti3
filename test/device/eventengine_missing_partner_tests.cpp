#include <catch2/catch_test_macros.hpp>

#include <sti/LocalDevice.h>
#include <sti/device/DeviceCollection.h>
#include <sti/device/DeviceID.h>
#include <sti/engine/EngineJobID.h>
#include <sti/engine/EngineID.h>
#include <sti/engine/EventEngineScheduler.h>
#include <sti/engine/EventEngineJob.h>
#include <sti/engine/EventEngineJobList.h>
#include <sti/engine/AddSequenceStatus.h>
#include <sti/engine/ParseJobStatus.h>
#include <sti/engine/ParseResult.h>
#include <sti/engine/PlayJobStatus.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/RawEventGroup.h>
#include <sti/engine/RawEventTarget.h>
#include <sti/engine/RawEventTargetChannel.h>
#include <sti/engine/Shot.h>
#include <sti/engine/ShotConfig.h>
#include <sti/engine/SequenceID.h>
#include <sti/engine/SynchronousEvent.h>
#include <sti/utils/Distributer.h>
#include <sti/utils/Configuration.h>
#include <sti/utils/MixedValue.h>

#include "EventEngine.h"
#include "LocalEventEngineScheduler.h"
#include "MasterTrigger.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <mutex>
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
using STI::Engine::EngineID;
using STI::Engine::EngineJobStatus;
using STI::Engine::EngineParsingMessage;
using STI::Engine::EnginePlayingMessage;
using STI::Engine::EventEngineScheduler;
using STI::Engine::EventEngineJob;
using STI::Engine::EventEngineJobList;
using STI::Engine::EventEngineJobType;
using STI::Engine::LocalEventEngineScheduler;
using STI::Engine::MasterTrigger;
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

Configuration makeFastPlaybackTimeoutConfig(const std::string& testName)
{
    auto config = makeDeviceConfig(testName);
    config.set("EngineManager", "PlayReady Timeout ms", 50);
    config.set("EngineManager", "Trigger Timeout ms", 50);
    config.set("EngineManager", "PlayComplete Grace ms", 50);
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

class BlockingBeforePlayEvent : public SynchronousEvent
{
public:
    BlockingBeforePlayEvent(double time, int& loadCount, int& playCount)
        : SynchronousEvent(time), loadCount(loadCount), playCount(playCount) {}

    void waitBeforePlay() override
    {
        std::unique_lock<std::mutex> lock(blockMutex);
        blockCondition.wait(lock, [this]() { return stopRequested; });
    }

    void loadEvent() override { ++loadCount; }
    void playEvent() override { ++playCount; }
    void collectMeasurementData() override {}
    void stopEvent() override
    {
        std::unique_lock<std::mutex> lock(blockMutex);
        stopRequested = true;
        blockCondition.notify_all();
    }
    void pauseEvent() override {}
    void unpauseEvent(bool) override {}

private:
    int& loadCount;
    int& playCount;
    bool stopRequested = false;
    std::mutex blockMutex;
    std::condition_variable blockCondition;
};

class FakePlayReadyEngine : public STI::Engine::EventEngine
{
public:
    explicit FakePlayReadyEngine(const DeviceID& deviceID)
        : deviceID(deviceID) {}

    void play(EventEngineJob&) override {}
    void play(const EngineJobID&, const std::shared_ptr<STI::Engine::TriggerCallback>&, bool) override {}
    void trigger() override {}
    void trigger(const DeviceID&) override {}
    void stop() override { stopped = true; }
    void pause() override {}
    void unpause(bool) override {}
    void clear() override {}

    DeviceID getDeviceID() const override { return deviceID; }
    STI::Engine::EngineState getState() const override
    {
        return stopped ? STI::Engine::EngineState::Parsed : STI::Engine::EngineState::PlayReady;
    }

    std::shared_ptr<STI::Engine::ParsedDependencyTree> getParsedTree() const override { return {}; }
    bool getParseResult(const ParseID&, std::shared_ptr<ParseResult>& parseResult) const override
    {
        parseResult.reset();
        return false;
    }

private:
    DeviceID deviceID;
    bool stopped = false;
};

enum class PlayJobInterception
{
    None,
    Drop,
    FakePlayReadyNoArm
};

class StallingScheduler : public EventEngineScheduler
{
public:
    StallingScheduler(const std::shared_ptr<LocalEventEngineScheduler>& realScheduler, LocalDevice* device)
        : realScheduler(realScheduler), device(device) {}

    STI::Engine::ParseJobStatus parse(const std::shared_ptr<STI::Engine::Shot>& shot) override
    {
        return realScheduler->parse(shot);
    }

    STI::Engine::ParseJobStatus parse(const std::shared_ptr<STI::Engine::Shot>& shot, const STI::Engine::SequenceEntryID& sequenceEntryID) override
    {
        return realScheduler->parse(shot, sequenceEntryID);
    }

    STI::Engine::ParseJobStatus parse(const std::shared_ptr<STI::Engine::Shot>& shot, const STI::Engine::SequenceID& sequenceID) override
    {
        return realScheduler->parse(shot, sequenceID);
    }

    STI::Engine::PlayJobStatus play(const ParseID& parseID, const STI::Engine::EngineJobSourceID& source) override
    {
        return realScheduler->play(parseID, source);
    }

    STI::Engine::AddSequenceStatus addSequence(const std::shared_ptr<STI::Engine::Sequence>& sequence, const STI::Engine::EngineJobSourceID& source) override
    {
        return realScheduler->addSequence(sequence, source);
    }

    void closeSequence(const STI::Engine::SequenceID& seqid) override { realScheduler->closeSequence(seqid); }
    void cancelSequence(const STI::Engine::SequenceID& seqid) override { realScheduler->cancelSequence(seqid); }

    EngineJobStatus getStatus(const ParseID& pid) override { return realScheduler->getStatus(pid); }
    EngineJobStatus getStatus(const ShotID& sid) override { return realScheduler->getStatus(sid); }
    EngineJobStatus getStatus(const STI::Engine::SequenceID& seqID) override { return realScheduler->getStatus(seqID); }

    bool getDependencyParser(std::shared_ptr<STI::Engine::EventEngineDependencyParser>& dependencyParser) override
    {
        return realScheduler->getDependencyParser(dependencyParser);
    }

    bool getJob(const EngineJobID& id, std::shared_ptr<EventEngineJob>& job) const override
    {
        return realScheduler->getJob(id, job);
    }

    void addJob(const std::shared_ptr<EventEngineJob>& newJob) override
    {
        if (newJob != nullptr && newJob->getJobID().type == EventEngineJobType::Play) {
            if (playJobInterception == PlayJobInterception::Drop) {
                stalledJobs.push_back(newJob);
                return;
            }
            if (playJobInterception == PlayJobInterception::FakePlayReadyNoArm) {
                sendFakePlayReady(newJob);
                return;
            }
        }

        realScheduler->addJob(newJob);
    }

    void sendFakePlayReady(const std::shared_ptr<EventEngineJob>& newJob)
    {
        if (device == nullptr || newJob == nullptr) {
            return;
        }

        if (fakeEngine == nullptr) {
            fakeEngine = std::make_shared<FakePlayReadyEngine>(device->getID());
        }

        auto playReadyMessage = std::make_shared<STI::Device::EngineSchedulerMessage>(
            device->getID(),
            STI::Device::EngineSchedulerMessage::SchedulerMessageType::PlayReady);
        playReadyMessage->jobID = newJob->getJobID();
        playReadyMessage->engineState = STI::Engine::EngineState::PlayReady;
        playReadyMessage->setEngine(fakeEngine);

        device->sendMessage(playReadyMessage);
    }

    void cancelJob(const EngineJobID& jobID) override { realScheduler->cancelJob(jobID); }
    void cancelAll() override { realScheduler->cancelAll(); }

    std::set<EngineJobID> getJobIDs(const EventEngineJobList& jobListType) const override
    {
        return realScheduler->getJobIDs(jobListType);
    }

    std::vector<std::shared_ptr<EventEngineJob>> getJobs(const EventEngineJobList& jobListType) const override
    {
        return realScheduler->getJobs(jobListType);
    }

    std::shared_ptr<STI::Engine::Shot> createShot(const ShotConfig& shotConfig, const std::shared_ptr<RawEventGroup>& eventGroup) override
    {
        return realScheduler->createShot(shotConfig, eventGroup);
    }

    void setEngineFactory(const std::shared_ptr<STI::Engine::EventEngineFactory>& engineFactory) override
    {
        realScheduler->setEngineFactory(engineFactory);
    }

    void getEngineIDs(std::set<EngineID>& engineIDs) const override { realScheduler->getEngineIDs(engineIDs); }
    STI::Engine::EngineState getEngineState(const EngineID& engineID) const override { return realScheduler->getEngineState(engineID); }
    void getEngineStates(std::map<EngineID, STI::Engine::EngineState>& engineStates) const override { realScheduler->getEngineStates(engineStates); }
    void clearEngine(const EngineID& engineID) override { realScheduler->clearEngine(engineID); }
    void stopEngine(const EngineID& engineID) override { realScheduler->stopEngine(engineID); }

    bool getParseResult(const ParseID& parseID, std::shared_ptr<ParseResult>& parseResult) const override
    {
        return realScheduler->getParseResult(parseID, parseResult);
    }

    bool getShotResult(const ShotID& shotID, std::shared_ptr<STI::Engine::ShotResult>& shotResult) const override
    {
        return realScheduler->getShotResult(shotID, shotResult);
    }

    bool getLastParseResult(const EngineID& engineID, std::shared_ptr<ParseResult>& parseResult) const override
    {
        return realScheduler->getLastParseResult(engineID, parseResult);
    }

    bool getLastShotResult(const EngineID& engineID, std::shared_ptr<STI::Engine::ShotResult>& shotResult) const override
    {
        return realScheduler->getLastShotResult(engineID, shotResult);
    }

    PlayJobInterception playJobInterception = PlayJobInterception::None;
    std::vector<std::shared_ptr<EventEngineJob>> stalledJobs;

private:
    std::shared_ptr<LocalEventEngineScheduler> realScheduler;
    LocalDevice* device;
    std::shared_ptr<FakePlayReadyEngine> fakeEngine;
};

class PartnerGeneratingDevice : public LocalDevice
{
public:
    PartnerGeneratingDevice(const std::string& name, unsigned short module, const std::string& targetServer)
        : PartnerGeneratingDevice(name, module, targetServer, makeDeviceConfig(name))
    {
    }

    PartnerGeneratingDevice(const std::string& name, unsigned short module, const std::string& targetServer, const Configuration& config)
        : LocalDevice(name, "127.0.0.1", module, targetServer, config)
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
            if (blockBeforePlay) {
                synchedEvents.push_back(std::make_shared<BlockingBeforePlayEvent>(tuple.first, loadCount, playCount));
            }
            else {
                synchedEvents.push_back(std::make_shared<CountingEvent>(tuple.first, loadCount, playCount));
            }

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
    bool blockBeforePlay = false;
    int loadCount = 0;
    int playCount = 0;
};

class PlayReadyStallDevice : public PartnerGeneratingDevice
{
public:
    PlayReadyStallDevice(const std::string& name, unsigned short module, const std::string& targetServer)
        : PartnerGeneratingDevice(name, module, targetServer) {}

    PlayReadyStallDevice(const std::string& name, unsigned short module, const std::string& targetServer, const Configuration& config)
        : PartnerGeneratingDevice(name, module, targetServer, config) {}

    using PartnerGeneratingDevice::getEngineScheduler;

    bool getEngineScheduler(std::shared_ptr<EventEngineScheduler>& scheduler) override
    {
        if (playJobInterception == PlayJobInterception::None) {
            return LocalDevice::getEngineScheduler(scheduler);
        }

        if (stallingScheduler == nullptr) {
            std::shared_ptr<LocalEventEngineScheduler> realScheduler;
            if (!LocalDevice::getEngineScheduler(realScheduler) || realScheduler == nullptr) {
                return false;
            }
            stallingScheduler = std::make_shared<StallingScheduler>(realScheduler, this);
        }

        stallingScheduler->playJobInterception = playJobInterception;
        scheduler = stallingScheduler;
        return true;
    }

    PlayJobInterception playJobInterception = PlayJobInterception::None;
    std::shared_ptr<StallingScheduler> stallingScheduler;
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
    for (int i = 0; i < 500; ++i) {
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

std::shared_ptr<EventEngineJob> waitForCompletedPlayJob(LocalEventEngineScheduler& scheduler, const ShotID& sid)
{
    for (int i = 0; i < 500; ++i) {
        auto job = findCompletedPlayJob(scheduler, sid);
        if (job != nullptr) {
            return job;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return findCompletedPlayJob(scheduler, sid);
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

TEST_CASE("MasterTrigger bounded arm wait reports pending devices")
{
    DeviceID triggerID("Trigger", "127.0.0.1", 200, "root");
    DeviceID readyID("Ready", "127.0.0.1", 201, "root");
    DeviceID pendingID("Pending", "127.0.0.1", 202, "root");

    MasterTrigger trigger(triggerID);
    trigger.arm(std::vector<DeviceID>{readyID, pendingID});
    trigger.ready(readyID);

    std::vector<DeviceID> pending;
    CHECK_FALSE(trigger.waitForArmFor(std::chrono::milliseconds(20), pending));
    REQUIRE(pending.size() == 1);
    CHECK(pending.front() == pendingID);

    trigger.stop();
}

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

TEST_CASE("Server cancels play when owned target loses parsed engine before play")
{
    auto server = std::make_shared<PartnerGeneratingDevice>("RobustServer", 80, "root");
    auto target = std::make_shared<PartnerGeneratingDevice>("RobustTarget", 81, server->getID().getID());

    auto distributer = distributeDevices({server, target});

    std::shared_ptr<DeviceCollection> collection;
    server->getCollection(collection);
    REQUIRE(collection != nullptr);
    REQUIRE(collection->contains(target->getID()));

    auto scheduler = schedulerFor(*server);
    auto shot = makeShot(*scheduler, target->getID());

    auto parseStatus = scheduler->parse(shot);
    REQUIRE(waitForParseTerminal(*scheduler, parseStatus.pid) == EngineJobStatus::Completed);
    requireConcreteParse(*scheduler, parseStatus.pid);

    auto targetScheduler = schedulerFor(*target);
    targetScheduler->clearEngine(EngineID(1));

    auto playStatus = scheduler->play(parseStatus.pid, shot->getShotConfig().jobSourceID);
    REQUIRE(waitForShotTerminal(*scheduler, playStatus.sid) == EngineJobStatus::Canceled);

    auto playJob = findCompletedPlayJob(*scheduler, playStatus.sid);
    REQUIRE(playJob != nullptr);
    CHECK(hasPlayError(playJob->getPlayMessages(), "Owned device state invalid"));
    CHECK(target->loadCount == 0);
    CHECK(target->playCount == 0);
}

TEST_CASE("Server cancels play when owned target never reports PlayReady")
{
    auto serverConfig = makeFastPlaybackTimeoutConfig("PlayReadyTimeoutServer");
    auto server = std::make_shared<PartnerGeneratingDevice>("PlayReadyTimeoutServer", 90, "root", serverConfig);
    auto target = std::make_shared<PlayReadyStallDevice>("PlayReadyTimeoutTarget", 91, server->getID().getID());

    auto distributer = distributeDevices({server, target});

    auto scheduler = schedulerFor(*server);
    auto shot = makeShot(*scheduler, target->getID());

    auto parseStatus = scheduler->parse(shot);
    REQUIRE(waitForParseTerminal(*scheduler, parseStatus.pid) == EngineJobStatus::Completed);
    requireConcreteParse(*scheduler, parseStatus.pid);

    target->playJobInterception = PlayJobInterception::Drop;

    auto start = std::chrono::steady_clock::now();
    auto playStatus = scheduler->play(parseStatus.pid, shot->getShotConfig().jobSourceID);
    REQUIRE(waitForShotTerminal(*scheduler, playStatus.sid) == EngineJobStatus::Canceled);
    CHECK(std::chrono::steady_clock::now() - start < std::chrono::seconds(1));

    auto playJob = waitForCompletedPlayJob(*scheduler, playStatus.sid);
    REQUIRE(playJob != nullptr);
    CHECK(hasPlayError(playJob->getPlayMessages(), "Owned device PlayReady timeout"));
    CHECK(target->loadCount == 0);
    CHECK(target->playCount == 0);
}

TEST_CASE("Server cancels play when owned target never reports PlayComplete")
{
    auto serverConfig = makeFastPlaybackTimeoutConfig("PlayCompleteTimeoutServer");
    auto server = std::make_shared<PartnerGeneratingDevice>("PlayCompleteTimeoutServer", 92, "root", serverConfig);
    auto target = std::make_shared<PartnerGeneratingDevice>("PlayCompleteTimeoutTarget", 93, server->getID().getID());
    target->blockBeforePlay = true;

    auto distributer = distributeDevices({server, target});

    auto scheduler = schedulerFor(*server);
    auto shot = makeShot(*scheduler, target->getID());

    auto parseStatus = scheduler->parse(shot);
    REQUIRE(waitForParseTerminal(*scheduler, parseStatus.pid) == EngineJobStatus::Completed);
    requireConcreteParse(*scheduler, parseStatus.pid);

    auto start = std::chrono::steady_clock::now();
    auto playStatus = scheduler->play(parseStatus.pid, shot->getShotConfig().jobSourceID);
    REQUIRE(waitForShotTerminal(*scheduler, playStatus.sid) == EngineJobStatus::Canceled);
    CHECK(std::chrono::steady_clock::now() - start < std::chrono::seconds(1));

    auto playJob = waitForCompletedPlayJob(*scheduler, playStatus.sid);
    REQUIRE(playJob != nullptr);
    CHECK(hasPlayError(playJob->getPlayMessages(), "Owned device PlayComplete timeout"));
    CHECK(target->loadCount == 1);
    CHECK(target->playCount == 0);
}

TEST_CASE("Server cancels play when owned target reports PlayReady but never arms")
{
    auto serverConfig = makeFastPlaybackTimeoutConfig("TriggerTimeoutServer");
    auto server = std::make_shared<PartnerGeneratingDevice>("TriggerTimeoutServer", 94, "root", serverConfig);
    auto target = std::make_shared<PlayReadyStallDevice>("TriggerTimeoutTarget", 95, server->getID().getID());

    auto distributer = distributeDevices({server, target});

    auto scheduler = schedulerFor(*server);
    auto shot = makeShot(*scheduler, target->getID());

    auto parseStatus = scheduler->parse(shot);
    REQUIRE(waitForParseTerminal(*scheduler, parseStatus.pid) == EngineJobStatus::Completed);
    requireConcreteParse(*scheduler, parseStatus.pid);

    target->playJobInterception = PlayJobInterception::FakePlayReadyNoArm;

    auto start = std::chrono::steady_clock::now();
    auto playStatus = scheduler->play(parseStatus.pid, shot->getShotConfig().jobSourceID);
    REQUIRE(waitForShotTerminal(*scheduler, playStatus.sid) == EngineJobStatus::Canceled);
    CHECK(std::chrono::steady_clock::now() - start < std::chrono::seconds(1));

    auto playJob = waitForCompletedPlayJob(*scheduler, playStatus.sid);
    REQUIRE(playJob != nullptr);
    CHECK(hasPlayError(playJob->getPlayMessages(), "Owned device trigger timeout"));
    CHECK(target->loadCount == 0);
    CHECK(target->playCount == 0);
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
