#include <catch2/catch_test_macros.hpp>

#include "../../src/device/src/LocalPostProcessingManager.h"
#include "../../src/device/src/LocalDeviceMessageDispatcher.h"
#include "../../src/device/src/LocalDeviceMessageHandler.h"

#include <sti/device/DeviceMessage.h>
#include <sti/device/DeviceMessageListener.h>
#include <sti/device/DeviceMessageListenerGroup.h>
#include <sti/device/DeviceID.h>
#include <sti/device/PersistenceManager.h>
#include <sti/engine/ShotID.h>
#include <sti/engine/ShotResult.h>
#include <sti/utils/MetaData.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>
#include <vector>

using STI::Device::LocalPostProcessingManager;
using STI::Device::LocalDeviceMessageDispatcher;
using STI::Device::LocalDeviceMessageHandler;
using STI::Device::DeviceMessageListener;
using STI::Device::DeviceMessageListenerGroup;
using STI::Device::AbstractMessageListenerGroup;
using STI::Device::PersistenceManager;
using STI::Device::PostProcessingCompleteMessage;
using STI::Device::PostProcessingStatus;
using STI::Device::DeviceID;
using STI::Engine::ShotID;
using STI::Engine::ShotResult;
using STI::Utils::MetaData;
using STI::Utils::MixedValue;


namespace
{

DeviceID makeDeviceID() { return DeviceID("Analysis", "localhost", 0); }

std::shared_ptr<ShotResult> makeShotResult()
{
    std::set<DeviceID> ownedIDs;
    return std::make_shared<ShotResult>(makeDeviceID(), ownedIDs);
}

//Minimal PersistenceManager test double: getShotResult returns the configured
//result (so the worker can pull it); everything else is an inert stub.
class StubPersistenceManager : public PersistenceManager
{
public:
    std::shared_ptr<ShotResult> shotResult;   //pulled by the worker; null => "not found"

    bool findShot(const ShotID&) override { return shotResult != nullptr; }
    bool getParseResult(const STI::Engine::ParseID&, std::shared_ptr<STI::Engine::ParseResult>&) override { return false; }
    bool getShotResult(const ShotID&, std::shared_ptr<ShotResult>& result) override
    {
        result = shotResult;
        return result != nullptr;
    }
    bool getSequenceResult(const STI::Engine::SequenceID&, std::shared_ptr<STI::Engine::SequenceResult>&) override { return false; }
    bool saveShot(const ShotID&, const std::shared_ptr<STI::Engine::FullShotResult>&, bool) override { return false; }
    STI::Engine::ShotResultRecord transferResults(const std::shared_ptr<STI::Engine::ResultsCollector>&) override { return STI::Engine::ShotResultRecord(); }
    void setResultsCollectorFactory(const std::shared_ptr<STI::Engine::ResultsCollectorFactory>&) override {}
    bool getMeasurements(const ShotID&, std::shared_ptr<STI::Engine::MeasurementMap>&) override { return false; }
    void setFileHolderFactory(const std::shared_ptr<STI::Utils::FileHolderFactory>&) override {}
    void setVirtualFileServerFactory(const std::shared_ptr<STI::Utils::VirtualFileServerFactory>&) override {}
    void setFileServer(const std::shared_ptr<STI::Utils::FileServer>&) override {}
    bool getFileServer(std::shared_ptr<STI::Utils::FileServer>&) override { return false; }
    std::shared_ptr<STI::Utils::VirtualFileServer> makeVirtualFileServer() override { return nullptr; }
    std::string getBasePath() const override { return ""; }
    std::string getTemporaryPath() const override { return ""; }
    void addSequence(const std::shared_ptr<STI::Engine::SequenceResult>&) override {}
    bool updateSequence(const STI::Engine::SequenceEntryID&, const ShotID&, const STI::Engine::EngineJobStatus&, bool) override { return false; }
    bool saveSequence(const std::shared_ptr<STI::Engine::SequenceResult>&, bool) override { return false; }
    std::shared_ptr<STI::Utils::FileHolder> makeFileHolder(const std::string&, const std::string&) override { return nullptr; }
    std::shared_ptr<STI::Utils::FileHolder> makeVirtualFileHolder(const STI::Utils::FileID&) override { return nullptr; }
    std::shared_ptr<STI::Utils::FileHolder> makeVirtualFileHolder(const std::shared_ptr<STI::Utils::VirtualFileHolder>&) override { return nullptr; }
};

std::shared_ptr<StubPersistenceManager> makeStubPersistence(bool withResult = true)
{
    auto pm = std::make_shared<StubPersistenceManager>();
    if (withResult) {
        pm->shotResult = makeShotResult();
    }
    return pm;
}

class CompleteRecorder : public DeviceMessageListener<PostProcessingCompleteMessage>
{
public:
    void handleMessage(const std::shared_ptr<PostProcessingCompleteMessage>& mess) override
    {
        std::unique_lock<std::mutex> lock(mutex);
        received.push_back(mess);
        condition.notify_all();
    }

    bool waitFor(std::size_t count, std::chrono::milliseconds timeout)
    {
        std::unique_lock<std::mutex> lock(mutex);
        return condition.wait_for(lock, timeout, [&] { return received.size() >= count; });
    }

    std::mutex mutex;
    std::condition_variable condition;
    std::vector<std::shared_ptr<PostProcessingCompleteMessage>> received;
};

//Wire a recorder so it receives messages dispatched by `dispatcher` for `id`.
std::shared_ptr<CompleteRecorder> attachRecorder(
    const std::shared_ptr<LocalDeviceMessageDispatcher>& dispatcher, const DeviceID& id)
{
    auto handler = std::make_shared<LocalDeviceMessageHandler>();
    dispatcher->addMessageHandler(id, handler);

    auto listenerGroup = std::make_shared<DeviceMessageListenerGroup<PostProcessingCompleteMessage>>();
    auto recorder = std::make_shared<CompleteRecorder>();
    listenerGroup->addListener(
        STI::Device::DeviceMessageListenerID(PostProcessingCompleteMessage::getMessageClassType(), "pp-test"),
        std::static_pointer_cast<DeviceMessageListener<PostProcessingCompleteMessage>>(recorder));

    std::shared_ptr<AbstractMessageListenerGroup> abstractGroup = listenerGroup;
    handler->addListenerGroup(PostProcessingCompleteMessage::getMessageClassType(), abstractGroup);

    return recorder;
}

} //namespace


TEST_CASE("LocalPostProcessingManager: requestPostProcessing returns false for unregistered target", "[postprocessing][localdevice]") {
    auto dispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    LocalPostProcessingManager manager(makeDeviceID(), dispatcher, nullptr, nullptr);

    CHECK_FALSE(manager.requestPostProcessing("missing", ShotID(), makeDeviceID(), MetaData()));

    manager.stop();
}

TEST_CASE("LocalPostProcessingManager: getPostProcessingTargets reports registered targets", "[postprocessing][localdevice]") {
    auto dispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    LocalPostProcessingManager manager(makeDeviceID(), dispatcher, nullptr, nullptr);

    manager.addPostProcessingTarget("fit", [](const std::shared_ptr<ShotResult>&, const MetaData&) { return MetaData(); }, "Gaussian fit");

    auto targets = manager.getPostProcessingTargets();
    REQUIRE(targets.size() == 1);
    CHECK(targets.front().name == "fit");
    CHECK(targets.front().description == "Gaussian fit");

    manager.stop();
}

TEST_CASE("LocalPostProcessingManager: success path pulls the ShotResult and broadcasts results", "[postprocessing][localdevice]") {
    auto dispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    auto recorder = attachRecorder(dispatcher, makeDeviceID());

    auto persistence = makeStubPersistence();
    LocalPostProcessingManager manager(makeDeviceID(), dispatcher, nullptr, persistence);

    std::atomic<bool> gotShotResult{false};
    manager.addPostProcessingTarget("fit", [&](const std::shared_ptr<ShotResult>& shotResult, const MetaData& options) {
        gotShotResult = (shotResult != nullptr);
        MetaData result;
        result.addMetaData("amplitude", MixedValue(3.5));
        return result;
    });

    //shotOwnerID == this device, so the worker resolves the local PersistenceManager.
    REQUIRE(manager.requestPostProcessing("fit", ShotID(), makeDeviceID(), MetaData()));
    REQUIRE(recorder->waitFor(1, std::chrono::milliseconds(2000)));

    CHECK(gotShotResult.load());

    auto mess = recorder->received.front();
    CHECK(mess->targetName == "fit");
    CHECK(mess->status == PostProcessingStatus::Success);
    CHECK(mess->errorMessage.empty());
    CHECK(mess->results.contains("amplitude"));
    CHECK(mess->results.getMetaData("amplitude").getDouble() == 3.5);

    manager.stop();
}

TEST_CASE("LocalPostProcessingManager: missing shot result aborts with Failed and no callback", "[postprocessing][localdevice]") {
    auto dispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    auto recorder = attachRecorder(dispatcher, makeDeviceID());

    //Owner is reachable (this device) but its PersistenceManager has no result.
    auto persistence = makeStubPersistence(/*withResult=*/false);
    LocalPostProcessingManager manager(makeDeviceID(), dispatcher, nullptr, persistence);

    std::atomic<bool> callbackRan{false};
    manager.addPostProcessingTarget("fit", [&](const std::shared_ptr<ShotResult>&, const MetaData&) {
        callbackRan = true;
        return MetaData();
    });

    REQUIRE(manager.requestPostProcessing("fit", ShotID(), makeDeviceID(), MetaData()));
    REQUIRE(recorder->waitFor(1, std::chrono::milliseconds(2000)));

    CHECK_FALSE(callbackRan.load());

    auto mess = recorder->received.front();
    CHECK(mess->status == PostProcessingStatus::Failed);
    CHECK(mess->errorMessage.find("not found") != std::string::npos);

    manager.stop();
}

TEST_CASE("LocalPostProcessingManager: unreachable owner aborts with Failed and no callback", "[postprocessing][localdevice]") {
    auto dispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    auto recorder = attachRecorder(dispatcher, makeDeviceID());

    //No collection and a different owner ID: the owner cannot be resolved.
    LocalPostProcessingManager manager(makeDeviceID(), dispatcher, nullptr, nullptr);

    std::atomic<bool> callbackRan{false};
    manager.addPostProcessingTarget("fit", [&](const std::shared_ptr<ShotResult>&, const MetaData&) {
        callbackRan = true;
        return MetaData();
    });

    DeviceID otherOwner("OtherOwner", "localhost", 1);
    REQUIRE(manager.requestPostProcessing("fit", ShotID(), otherOwner, MetaData()));
    REQUIRE(recorder->waitFor(1, std::chrono::milliseconds(2000)));

    CHECK_FALSE(callbackRan.load());

    auto mess = recorder->received.front();
    CHECK(mess->status == PostProcessingStatus::Failed);
    CHECK(mess->errorMessage.find("partner") != std::string::npos);

    manager.stop();
}

TEST_CASE("LocalPostProcessingManager: exception path reports failure", "[postprocessing][localdevice]") {
    auto dispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    auto recorder = attachRecorder(dispatcher, makeDeviceID());

    auto persistence = makeStubPersistence();
    LocalPostProcessingManager manager(makeDeviceID(), dispatcher, nullptr, persistence);

    manager.addPostProcessingTarget("boom", [](const std::shared_ptr<ShotResult>&, const MetaData&) -> MetaData {
        throw std::runtime_error("kaboom");
    });

    REQUIRE(manager.requestPostProcessing("boom", ShotID(), makeDeviceID(), MetaData()));
    REQUIRE(recorder->waitFor(1, std::chrono::milliseconds(2000)));

    auto mess = recorder->received.front();
    CHECK(mess->targetName == "boom");
    CHECK(mess->status == PostProcessingStatus::Failed);
    CHECK(mess->errorMessage == "kaboom");

    manager.stop();
}

TEST_CASE("LocalPostProcessingManager: multiple requests for one target queue independently", "[postprocessing][localdevice]") {
    auto dispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    auto recorder = attachRecorder(dispatcher, makeDeviceID());

    auto persistence = makeStubPersistence();
    LocalPostProcessingManager manager(makeDeviceID(), dispatcher, nullptr, persistence);

    manager.addPostProcessingTarget("fit", [](const std::shared_ptr<ShotResult>&, const MetaData&) { return MetaData(); });

    REQUIRE(manager.requestPostProcessing("fit", ShotID(), makeDeviceID(), MetaData()));
    REQUIRE(manager.requestPostProcessing("fit", ShotID(), makeDeviceID(), MetaData()));
    REQUIRE(recorder->waitFor(2, std::chrono::milliseconds(2000)));

    CHECK(recorder->received.size() == 2);

    manager.stop();
}
