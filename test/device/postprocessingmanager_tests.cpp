#include <catch2/catch_test_macros.hpp>

#include "../../src/device/src/LocalPostProcessingManager.h"
#include "../../src/device/src/LocalDeviceMessageDispatcher.h"
#include "../../src/device/src/LocalDeviceMessageHandler.h"

#include <sti/device/DeviceMessage.h>
#include <sti/device/DeviceMessageListener.h>
#include <sti/device/DeviceMessageListenerGroup.h>
#include <sti/device/DeviceID.h>
#include <sti/engine/ShotID.h>
#include <sti/utils/MetaData.h>

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <vector>

using STI::Device::LocalPostProcessingManager;
using STI::Device::LocalDeviceMessageDispatcher;
using STI::Device::LocalDeviceMessageHandler;
using STI::Device::DeviceMessageListener;
using STI::Device::DeviceMessageListenerGroup;
using STI::Device::AbstractMessageListenerGroup;
using STI::Device::PostProcessingCompleteMessage;
using STI::Device::PostProcessingStatus;
using STI::Device::DeviceID;
using STI::Engine::ShotID;
using STI::Utils::MetaData;
using STI::Utils::MixedValue;


namespace
{

DeviceID makeDeviceID() { return DeviceID("Analysis", "localhost", 0); }

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

    manager.addPostProcessingTarget("fit", [](const ShotID&, const MetaData&) { return MetaData(); }, "Gaussian fit");

    auto targets = manager.getPostProcessingTargets();
    REQUIRE(targets.size() == 1);
    CHECK(targets.front().name == "fit");
    CHECK(targets.front().description == "Gaussian fit");

    manager.stop();
}

TEST_CASE("LocalPostProcessingManager: success path broadcasts results", "[postprocessing][localdevice]") {
    auto dispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    auto recorder = attachRecorder(dispatcher, makeDeviceID());

    LocalPostProcessingManager manager(makeDeviceID(), dispatcher, nullptr, nullptr);

    manager.addPostProcessingTarget("fit", [](const ShotID&, const MetaData& options) {
        MetaData result;
        result.addMetaData("amplitude", MixedValue(3.5));
        return result;
    });

    REQUIRE(manager.requestPostProcessing("fit", ShotID(), makeDeviceID(), MetaData()));
    REQUIRE(recorder->waitFor(1, std::chrono::milliseconds(2000)));

    auto mess = recorder->received.front();
    CHECK(mess->targetName == "fit");
    CHECK(mess->status == PostProcessingStatus::Success);
    CHECK(mess->errorMessage.empty());
    CHECK(mess->results.contains("amplitude"));
    CHECK(mess->results.getMetaData("amplitude").getDouble() == 3.5);

    manager.stop();
}

TEST_CASE("LocalPostProcessingManager: exception path reports failure", "[postprocessing][localdevice]") {
    auto dispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    auto recorder = attachRecorder(dispatcher, makeDeviceID());

    LocalPostProcessingManager manager(makeDeviceID(), dispatcher, nullptr, nullptr);

    manager.addPostProcessingTarget("boom", [](const ShotID&, const MetaData&) -> MetaData {
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

    LocalPostProcessingManager manager(makeDeviceID(), dispatcher, nullptr, nullptr);

    manager.addPostProcessingTarget("fit", [](const ShotID&, const MetaData&) { return MetaData(); });

    REQUIRE(manager.requestPostProcessing("fit", ShotID(), makeDeviceID(), MetaData()));
    REQUIRE(manager.requestPostProcessing("fit", ShotID(), makeDeviceID(), MetaData()));
    REQUIRE(recorder->waitFor(2, std::chrono::milliseconds(2000)));

    CHECK(recorder->received.size() == 2);

    manager.stop();
}
