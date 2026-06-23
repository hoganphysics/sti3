#include <catch2/catch_test_macros.hpp>

#include <sti/LocalDevice.h>
#include <sti/device/DeviceCollection.h>
#include <sti/device/DeviceID.h>
#include <sti/engine/EngineID.h>
#include <sti/engine/EngineJobID.h>
#include <sti/engine/EngineJobStatus.h>
#include <sti/engine/EventEngineScheduler.h>
#include <sti/engine/EventEngineJob.h>
#include <sti/engine/ParseJobStatus.h>
#include <sti/engine/ParseResult.h>
#include <sti/engine/PlayJobStatus.h>
#include <sti/engine/PostProcessTarget.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/RawEventGroup.h>
#include <sti/engine/RawEventTarget.h>
#include <sti/engine/RawEventTargetChannel.h>
#include <sti/engine/Shot.h>
#include <sti/engine/ShotConfig.h>
#include <sti/engine/ShotResult.h>
#include <sti/engine/SynchronousEvent.h>
#include <sti/utils/Distributer.h>
#include <sti/utils/Configuration.h>
#include <sti/utils/MetaData.h>
#include <sti/utils/MixedValue.h>

#include "LocalEventEngineScheduler.h"
#include "EventEngineDependencyTree.h"
#include "StackTrace.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <memory>
#include <string>
#include <thread>

using STI::Device::ChannelType;
using STI::Device::Device;
using STI::Device::DeviceCollection;
using STI::Device::DeviceID;
using STI::Device::LocalDevice;
using STI::Engine::EngineID;
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
using STI::Engine::PostProcessTarget;
using STI::Engine::RawEvent;
using STI::Engine::RawEventGroup;
using STI::Engine::RawEventMap;
using STI::Engine::RawEventTarget;
using STI::Engine::RawEventTargetChannel;
using STI::Engine::RawEventType;
using STI::Engine::ShotConfig;
using STI::Engine::ShotID;
using STI::Engine::StackTrace;
using STI::Engine::SynchronousEvent;
using STI::Engine::SynchronousEventVector;
using STI::Utils::Configuration;
using STI::Utils::Distributer;
using STI::Utils::MetaData;
using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;

namespace {

Configuration makeDeviceConfig(const std::string& testName)
{
    static std::atomic<unsigned> counter{0};

    auto root = std::filesystem::temp_directory_path()
        / ("sti3_postprocess_" + testName + "_" + std::to_string(counter++));

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

//Minimal playable device: one output channel and a counting event per parsed event.
class PlayingDevice : public LocalDevice
{
public:
    PlayingDevice(const std::string& name, unsigned short module, const std::string& targetServer)
        : PlayingDevice(name, module, targetServer, makeDeviceConfig(name)) {}

    PlayingDevice(const std::string& name, unsigned short module, const std::string& targetServer, const Configuration& config)
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
            synchedEvents.push_back(std::make_shared<CountingEvent>(tuple.first, loadCount, playCount));

            if (!partnerID.empty()) {
                for (auto& rawEvent : tuple.second) {
                    partner(partnerID).addEvent(tuple.first, partnerChannel, rawEvent.value(), rawEvent);
                }
            }
        }
    }

    DeviceID partnerID;
    int loadCount = 0;
    int playCount = 0;
};

std::shared_ptr<LocalEventEngineScheduler> schedulerFor(LocalDevice& device)
{
    std::shared_ptr<LocalEventEngineScheduler> scheduler;
    REQUIRE(device.getEngineScheduler(scheduler));
    REQUIRE(scheduler != nullptr);
    return scheduler;
}

std::unique_ptr<Distributer<DeviceID, Device>> distributeDevices(std::initializer_list<std::shared_ptr<LocalDevice>> devices)
{
    auto distributer = std::make_unique<Distributer<DeviceID, Device>>();
    for (auto& device : devices) {
        REQUIRE(distributer->add(device->getID(), device));
    }
    return distributer;
}

//Build a shot that plays one event on `playerID` and (optionally) declares a
//post-processing request against `target`.
std::shared_ptr<STI::Engine::Shot> makeShot(LocalEventEngineScheduler& scheduler, const DeviceID& playerID,
                                            const PostProcessTarget* target)
{
    ShotConfig config;
    config.jobSourceID.user = "postprocess-test";
    config.jobSourceID.machine = "localhost";

    auto group = std::make_shared<RawEventGroup>("root", "");
    group->addEvent(RawEventTarget(playerID, 0), 10.0, MixedValue(1.0), RawEventType::Play);

    if (target != nullptr) {
        group->addPostProcessRequest(*target, MetaData(), StackTrace());
    }

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

std::vector<EngineParsingMessage> parseMessagesFor(LocalEventEngineScheduler& scheduler, const ParseID& pid)
{
    std::shared_ptr<ParseResult> parseResult;
    REQUIRE(scheduler.getParseResult(pid, parseResult));
    REQUIRE(parseResult != nullptr);
    return parseResult->messages;
}

} // namespace


TEST_CASE("Resolvable post-processing target plays normally and dispatches the callback", "[postprocessing][eventengine]")
{
    auto player = std::make_shared<PlayingDevice>("PPPlayer", 1, "root");
    auto analysis = std::make_shared<PlayingDevice>("PPAnalysis", 2, player->getID().getID());

    std::atomic<int> ppCalls{0};
    std::atomic<bool> gotShotResult{false};
    analysis->addPostProcessingTarget("fit", [&](const std::shared_ptr<STI::Engine::ShotResult>& shotResult, const MetaData&) {
        gotShotResult = (shotResult != nullptr);
        ++ppCalls;
        return MetaData();
    });
    //The analysis device pulls the shot result from the owner, so declare the owner
    //as a partner (puts a direct reference to the owner in the analysis collection).
    analysis->addPartner(player->getID());

    auto distributer = distributeDevices({player, analysis});

    std::shared_ptr<DeviceCollection> collection;
    player->getCollection(collection);
    REQUIRE(collection != nullptr);
    REQUIRE(collection->contains(analysis->getID()));

    auto scheduler = schedulerFor(*player);

    //Concrete target (resolved device ID). Abstract/name-only targets are not yet
    //concretized (binding is WIP) and take the warning-and-skip path instead.
    PostProcessTarget target(STI::Engine::RawEventTargetDevice(analysis->getID()), "fit");
    auto shot = makeShot(*scheduler, player->getID(), &target);

    auto parseStatus = scheduler->parse(shot);
    REQUIRE(waitForParseTerminal(*scheduler, parseStatus.pid) == EngineJobStatus::Completed);

    auto messages = parseMessagesFor(*scheduler, parseStatus.pid);
    CHECK(countParsingMessages(messages, "Missing post-processing target") == 0);
    CHECK(countParsingMessages(messages, "Abstract Shot") == 0);

    auto playStatus = scheduler->play(parseStatus.pid, shot->getShotConfig().jobSourceID);
    REQUIRE(waitForShotTerminal(*scheduler, playStatus.sid) == EngineJobStatus::Completed);

    CHECK(player->playCount == 1);

    bool dispatched = false;
    for (int i = 0; i < 200 && !dispatched; ++i) {
        dispatched = (ppCalls.load() > 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    CHECK(dispatched);
    CHECK(ppCalls.load() == 1);
    CHECK(gotShotResult.load());   //worker pulled the owner's ShotResult and handed it to the callback
}

TEST_CASE("Parse extends the dependency tree with post-process-only target nodes", "[postprocessing][eventengine]")
{
    //The post-process target has no events, so it enters the dependency tree only
    //via the second (additive) getDependants pass over the post-process side-list.
    //Verifying the tree carries both the event owner and the analysis device proves
    //the second pass extended the pre-populated tree instead of replacing it.
    auto player = std::make_shared<PlayingDevice>("PPTreePlayer", 20, "root");
    auto analysis = std::make_shared<PlayingDevice>("PPTreeAnalysis", 21, player->getID().getID());
    auto distributer = distributeDevices({player, analysis});

    auto scheduler = schedulerFor(*player);

    PostProcessTarget target(STI::Engine::RawEventTargetDevice(analysis->getID()), "fit");
    auto shot = makeShot(*scheduler, player->getID(), &target);

    auto parseStatus = scheduler->parse(shot);
    REQUIRE(waitForParseTerminal(*scheduler, parseStatus.pid) == EngineJobStatus::Completed);

    std::shared_ptr<EventEngineJob> parseJob;
    REQUIRE(scheduler->getJob(EngineJobID(parseStatus.pid), parseJob));
    REQUIRE(parseJob != nullptr);

    std::shared_ptr<STI::Engine::EventEngineDependencyTree> tree;
    REQUIRE(parseJob->getDependencies(tree));
    REQUIRE(tree != nullptr);

    CHECK(tree->hasVertex(player->getID()));     //event-target pass (also the tree root)
    CHECK(tree->hasVertex(analysis->getID()));   //post-process pass extended the same tree
}

TEST_CASE("Post-processing request routes through a two-level hierarchy to a nested target", "[postprocessing][eventengine]")
{
    //Topology: player (job owner) -> subServer -> analysis. The analysis device is
    //reachable from the owner only via subServer (it names subServer as its server,
    //and subServer names player). The owner's collection does not contain analysis
    //directly, so the request must be forwarded one hop (player -> subServer) and then
    //delivered (subServer -> analysis). This exercises the recursive forwarding path.
    auto player = std::make_shared<PlayingDevice>("PPHPlayer", 10, "root");
    auto subServer = std::make_shared<PlayingDevice>("PPHServer", 11, player->getID().getID());
    auto analysis = std::make_shared<PlayingDevice>("PPHAnalysis", 12, subServer->getID().getID());

    std::atomic<int> ppCalls{0};
    std::atomic<bool> gotShotResult{false};
    analysis->addPostProcessingTarget("fit", [&](const std::shared_ptr<STI::Engine::ShotResult>& shotResult, const MetaData&) {
        gotShotResult = (shotResult != nullptr);
        ++ppCalls;
        return MetaData();
    });
    //The nested analysis device pulls the shot result directly from the owner, so
    //declare the owner as a partner regardless of the server hierarchy depth.
    analysis->addPartner(player->getID());

    auto distributer = distributeDevices({player, subServer, analysis});

    //The owner owns subServer but not the nested analysis device.
    std::shared_ptr<DeviceCollection> ownerCollection;
    player->getCollection(ownerCollection);
    REQUIRE(ownerCollection != nullptr);
    REQUIRE(ownerCollection->contains(subServer->getID()));
    REQUIRE_FALSE(ownerCollection->contains(analysis->getID()));

    //The sub-server owns the analysis device.
    std::shared_ptr<DeviceCollection> subServerCollection;
    subServer->getCollection(subServerCollection);
    REQUIRE(subServerCollection != nullptr);
    REQUIRE(subServerCollection->contains(analysis->getID()));

    auto scheduler = schedulerFor(*player);

    PostProcessTarget target(STI::Engine::RawEventTargetDevice(analysis->getID()), "fit");
    auto shot = makeShot(*scheduler, player->getID(), &target);

    auto parseStatus = scheduler->parse(shot);
    REQUIRE(waitForParseTerminal(*scheduler, parseStatus.pid) == EngineJobStatus::Completed);

    auto messages = parseMessagesFor(*scheduler, parseStatus.pid);
    CHECK(countParsingMessages(messages, "Missing post-processing target") == 0);
    CHECK(countParsingMessages(messages, "Abstract Shot") == 0);

    auto playStatus = scheduler->play(parseStatus.pid, shot->getShotConfig().jobSourceID);
    REQUIRE(waitForShotTerminal(*scheduler, playStatus.sid) == EngineJobStatus::Completed);

    CHECK(player->playCount == 1);

    //The forwarded request reaches the nested analysis device's worker.
    bool dispatched = false;
    for (int i = 0; i < 200 && !dispatched; ++i) {
        dispatched = (ppCalls.load() > 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    CHECK(dispatched);
    CHECK(ppCalls.load() == 1);
    CHECK(gotShotResult.load());   //pulled across the hierarchy from the owner (declared as partner)
}

TEST_CASE("Missing post-processing target warns but the shot still plays", "[postprocessing][eventengine]")
{
    auto player = std::make_shared<PlayingDevice>("PPWarnPlayer", 3, "root");
    auto distributer = distributeDevices({player});

    auto scheduler = schedulerFor(*player);

    PostProcessTarget target("NoSuchAnalysisDevice", "fit");
    auto shot = makeShot(*scheduler, player->getID(), &target);

    auto parseStatus = scheduler->parse(shot);
    REQUIRE(waitForParseTerminal(*scheduler, parseStatus.pid) == EngineJobStatus::Completed);

    auto messages = parseMessagesFor(*scheduler, parseStatus.pid);
    CHECK(countParsingMessages(messages, "Missing post-processing target") == 1);
    // Must NOT poison the shot via the abstract-shot path.
    CHECK(countParsingMessages(messages, "Abstract Shot") == 0);

    std::shared_ptr<EventEngineJob> parseJob;
    REQUIRE(scheduler->getJob(EngineJobID(parseStatus.pid), parseJob));
    REQUIRE(parseJob != nullptr);
    CHECK(parseJob->getMissingTargetIDs().empty());

    auto playStatus = scheduler->play(parseStatus.pid, shot->getShotConfig().jobSourceID);
    REQUIRE(waitForShotTerminal(*scheduler, playStatus.sid) == EngineJobStatus::Completed);

    CHECK(player->playCount == 1);

    auto playJob = findCompletedPlayJob(*scheduler, playStatus.sid);
    REQUIRE(playJob != nullptr);
    CHECK_FALSE(hasPlayError(playJob->getPlayMessages(), "Cannot Play Abstract Shot"));
}

TEST_CASE("Missing hard-timed device still hard-errors (separate from post-processing path)", "[postprocessing][eventengine]")
{
    auto player = std::make_shared<PlayingDevice>("PPRegressionPlayer", 4, "root");

    // A hard-timed partner target that is not on the network: the existing
    // abstract-shot machinery must still block play (independent of post-processing).
    DeviceID missingDevice("MissingHardTimedDevice", "127.0.0.1", 99, player->getID().getID());
    player->setPartnerTarget(missingDevice);

    auto distributer = distributeDevices({player});

    auto scheduler = schedulerFor(*player);

    auto shot = makeShot(*scheduler, player->getID(), nullptr);

    auto parseStatus = scheduler->parse(shot);
    REQUIRE(waitForParseTerminal(*scheduler, parseStatus.pid) == EngineJobStatus::Completed);

    auto messages = parseMessagesFor(*scheduler, parseStatus.pid);
    CHECK(countParsingMessages(messages, "Abstract Shot") == 1);
    CHECK(countParsingMessages(messages, "Missing post-processing target") == 0);

    auto playStatus = scheduler->play(parseStatus.pid, shot->getShotConfig().jobSourceID);
    REQUIRE(waitForShotTerminal(*scheduler, playStatus.sid) == EngineJobStatus::Canceled);

    auto playJob = findCompletedPlayJob(*scheduler, playStatus.sid);
    REQUIRE(playJob != nullptr);
    CHECK(hasPlayError(playJob->getPlayMessages(), "Cannot Play Abstract Shot"));
}
