#include <catch2/catch_test_macros.hpp>

#include <sti/device/DeviceMessage.h>
#include <sti/device/MessageGrouper.h>

#include <condition_variable>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace {

class DummyMessage : public STI::Device::GroupableMessage<DummyMessage> {
public:
    explicit DummyMessage(int value, bool groupable = true)
        : value(value), groupableFlag(groupable), appended(0) {}

    bool appendMessage(const DummyMessage& mess) override {
        value += mess.value;
        ++appended;
        return true;
    }

    bool groupable() const override { return groupableFlag; }
    DummyMessage& get() override { return *this; }

    int value;
    bool groupableFlag;
    int appended;
};

class RecordingGrouper : public STI::Device::MessageGrouper<DummyMessage> {
public:
    void dispatchMessage(const std::shared_ptr<DummyMessage>& mess) override {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            dispatched_.push_back(*mess);
        }
        cv_.notify_all();
    }

    bool waitForDispatches(std::size_t expected, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex_);
        return cv_.wait_for(lock, timeout, [&] { return dispatched_.size() >= expected; });
    }

    std::vector<DummyMessage> dispatched() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return dispatched_;
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::vector<DummyMessage> dispatched_;
};

class RecordingChannelUpdateGrouper : public STI::Device::MessageGrouper<STI::Device::ChannelUpdateMessage> {
public:
    void dispatchMessage(const std::shared_ptr<STI::Device::ChannelUpdateMessage>& mess) override {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            dispatched_.push_back(*mess);
        }
        cv_.notify_all();
    }

    bool waitForDispatches(std::size_t expected, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex_);
        return cv_.wait_for(lock, timeout, [&] { return dispatched_.size() >= expected; });
    }

    std::vector<STI::Device::ChannelUpdateMessage> dispatched() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return dispatched_;
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::vector<STI::Device::ChannelUpdateMessage> dispatched_;
};

} // namespace

TEST_CASE("MessageGrouper dispatches non-groupable messages immediately", "[device] [messagegrouper]") {
    RecordingGrouper grouper;
    grouper.setWarmup(20);
    grouper.setCooldown(20);
    grouper.start();

    auto message = std::make_shared<DummyMessage>(5, false);
    grouper.addMessage(message);

    REQUIRE(grouper.waitForDispatches(1, std::chrono::milliseconds(200)));
    auto dispatched = grouper.dispatched();
    REQUIRE(dispatched.size() == 1);
    CHECK(dispatched[0].value == 5);

    grouper.stop();
}

TEST_CASE("MessageGrouper groups messages during warmup", "[device] [messagegrouper]") {
    RecordingGrouper grouper;
    grouper.setWarmup(40);
    grouper.setCooldown(30);
    grouper.start();

    auto first = std::make_shared<DummyMessage>(1);
    grouper.addMessage(first);

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    grouper.addMessage(std::make_shared<DummyMessage>(2));

    REQUIRE(grouper.waitForDispatches(1, std::chrono::milliseconds(300)));
    auto dispatched = grouper.dispatched();
    REQUIRE(dispatched.size() == 1);
    CHECK(dispatched[0].value == 3);
    CHECK(dispatched[0].appended == 1);

    grouper.stop();
}

TEST_CASE("MessageGrouper keeps the newest channel value in grouped channel updates", "[device] [messagegrouper]") {
    RecordingChannelUpdateGrouper grouper;
    grouper.setWarmup(40);
    grouper.setCooldown(30);
    grouper.start();

    STI::Device::DeviceID source("MessageGrouperDevice", "127.0.0.1", 1);

    grouper.addMessage(std::make_shared<STI::Device::ChannelUpdateMessage>(source, 1, STI::Utils::MixedValue(10)));
    grouper.addMessage(std::make_shared<STI::Device::ChannelUpdateMessage>(source, 2, STI::Utils::MixedValue(20)));
    grouper.addMessage(std::make_shared<STI::Device::ChannelUpdateMessage>(source, 1, STI::Utils::MixedValue(30)));
    grouper.addMessage(STI::Device::ChannelUpdateMessage::makeMeasurementMessage(source, 1, STI::Utils::MixedValue(100)));
    grouper.addMessage(STI::Device::ChannelUpdateMessage::makeMeasurementMessage(source, 1, STI::Utils::MixedValue(300)));

    REQUIRE(grouper.waitForDispatches(1, std::chrono::milliseconds(300)));
    auto dispatched = grouper.dispatched();
    REQUIRE(dispatched.size() == 1);

    const auto& message = dispatched[0];
    REQUIRE(message.channelUpdateType == STI::Device::ChannelUpdateMessage::ChannelUpdateMessageType::ChannelValue);

    REQUIRE(message.channelValues.size() == 2);
    CHECK(message.channelValues.at(1) == STI::Utils::MixedValue(30));
    CHECK(message.channelValues.at(2) == STI::Utils::MixedValue(20));

    REQUIRE(message.measurementValues.size() == 1);
    CHECK(message.measurementValues.at(1) == STI::Utils::MixedValue(300));

    grouper.addMessage(std::make_shared<STI::Device::ChannelUpdateMessage>(source, 1, STI::Utils::MixedValue(40)));
    grouper.addMessage(std::make_shared<STI::Device::ChannelUpdateMessage>(source, 1, STI::Utils::MixedValue(50)));

    REQUIRE(grouper.waitForDispatches(2, std::chrono::milliseconds(500)));
    dispatched = grouper.dispatched();
    REQUIRE(dispatched.size() == 2);

    const auto& secondMessage = dispatched[1];
    REQUIRE(secondMessage.channelValues.size() == 1);
    CHECK(secondMessage.channelValues.at(1) == STI::Utils::MixedValue(50));
    CHECK(secondMessage.measurementValues.empty());

    grouper.stop();
}

TEST_CASE("MessageGrouper batches messages arriving during cooldown", "[device] [messagegrouper]") {
    RecordingGrouper grouper;
    grouper.setWarmup(25);
    grouper.setCooldown(60);
    grouper.start();

    grouper.addMessage(std::make_shared<DummyMessage>(4));
    REQUIRE(grouper.waitForDispatches(1, std::chrono::milliseconds(300)));

    // Add messages while the grouper is cooling; they should form the next batch.
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    grouper.addMessage(std::make_shared<DummyMessage>(2));
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    grouper.addMessage(std::make_shared<DummyMessage>(3));

    REQUIRE(grouper.waitForDispatches(2, std::chrono::milliseconds(400)));
    auto dispatched = grouper.dispatched();
    REQUIRE(dispatched.size() == 2);
    CHECK(dispatched[0].value == 4);
    CHECK(dispatched[1].value == 5);
    CHECK(dispatched[1].appended == 1);

    grouper.stop();
}

TEST_CASE("MessageGrouper stops without sending pending messages", "[device] [messagegrouper]") {
    RecordingGrouper grouper;
    grouper.setWarmup(50);
    grouper.setCooldown(50);
    grouper.start();

    grouper.addMessage(std::make_shared<DummyMessage>(7));
    grouper.stop();

    // Allow time for any unexpected dispatch to occur
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto dispatched = grouper.dispatched();
    CHECK(dispatched.empty());
}
