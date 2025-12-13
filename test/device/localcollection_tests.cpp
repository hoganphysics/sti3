#include <catch2/catch_test_macros.hpp>

#include <sti/utils/LocalCollection.h>
#include <sti/utils/Collector.h>
#include <sti/utils/SynchronizedMap.h>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

using STI::Utils::Collector;
using STI::Utils::LocalCollection;
using STI::Utils::LocalCollectionListener;
using STI::Utils::SynchronizedMapEvent;
using STI::Utils::SynchronizedMapPolicy;

namespace {
struct DummyBase {
    explicit DummyBase(int v) : value(v) {}
    int value;
};

struct DummyDerived : DummyBase {
    DummyDerived(int v, std::string n) : DummyBase(v), name(std::move(n)) {}
    std::string name;
};

class PrefixPolicy : public SynchronizedMapPolicy<std::string> {
public:
    explicit PrefixPolicy(std::string prefix) : prefix_(std::move(prefix)) {}
    bool include(const std::string& key) const override {
        return key.rfind(prefix_, 0) == 0;
    }
    bool replace(const std::string& oldKey, const std::string& newKey) const override {
        return oldKey == newKey;
    }

private:
    std::string prefix_;
};

class RecordingListener : public LocalCollectionListener<std::string> {
public:
    using EventType = SynchronizedMapEvent<std::string>::Type;

    void add(const std::string& id) override { record(id, EventType::Add); }
    void remove(const std::string& id) override { record(id, EventType::Remove); }
    void refresh() override { record("", EventType::Refresh); }

    bool waitFor(std::size_t expected, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return events.size() >= expected; });
    }

    std::vector<std::pair<std::string, EventType>> events;

private:
    void record(const std::string& id, EventType type) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            events.emplace_back(id, type);
        }
        cv.notify_all();
    }

    std::mutex mutex;
    std::condition_variable cv;
};

class DummyCollector : public Collector<std::string, DummyBase> {
public:
    DummyCollector() : collection(std::make_shared<LocalCollection<std::string, DummyBase>>()) {}

    void getCollection(std::shared_ptr<STI::Utils::Collection<std::string, DummyBase>>& out) override {
        out = collection;
    }

    std::shared_ptr<LocalCollection<std::string, DummyBase>> collection;
};
} // namespace

TEST_CASE("LocalCollection: add and get derived node") {
    LocalCollection<std::string, DummyBase> collection;

    auto derived = std::make_shared<DummyDerived>(7, "node");
    REQUIRE(collection.add("id1", derived));
    CHECK(collection.size() == 1);
    CHECK(collection.contains("id1"));

    std::shared_ptr<DummyBase> out;
    REQUIRE(collection.get("id1", out));
    REQUIRE(out != nullptr);
    CHECK(out->value == 7);

    std::set<std::string> ids;
    collection.getIDs(ids);
    REQUIRE(ids.size() == 1);
    CHECK(*ids.begin() == "id1");
}

TEST_CASE("LocalCollection: policy enforced at construction and cleanup") {
    auto policy = std::make_shared<PrefixPolicy>("ok");
    LocalCollection<std::string, DummyBase> collection(policy);

    auto okNode = std::make_shared<DummyBase>(1);
    auto badNode = std::make_shared<DummyBase>(2);

    CHECK(collection.add("ok.one", okNode));
    CHECK_FALSE(collection.add("bad", badNode));
    CHECK(collection.size() == 1);
    CHECK(collection.contains("ok.one"));

    collection.cleanup();  // should leave ok entry intact
    CHECK(collection.contains("ok.one"));
    CHECK(collection.size() == 1);
}

TEST_CASE("LocalCollection: listener receives events") {
    LocalCollection<std::string, DummyBase> collection;
    auto listener = std::make_shared<RecordingListener>();
    collection.addListener(listener);

    collection.add("a", std::make_shared<DummyBase>(1));
    collection.add("b", std::make_shared<DummyBase>(2));
    collection.remove("a");
    collection.clear();  // removes remaining and emits refresh

    REQUIRE(listener->waitFor(5, std::chrono::milliseconds(500)));

    using Type = SynchronizedMapEvent<std::string>::Type;
    std::vector<Type> observed;
    for (auto& evt : listener->events) {
        observed.push_back(evt.second);
    }
    std::vector<Type> expected{Type::Add, Type::Add, Type::Remove, Type::Remove, Type::Refresh};
    CHECK(observed == expected);
}

TEST_CASE("Collector interface returns collection") {
    DummyCollector collector;
    std::shared_ptr<STI::Utils::Collection<std::string, DummyBase>> basePtr;
    collector.getCollection(basePtr);
    REQUIRE(basePtr != nullptr);

    auto local = std::dynamic_pointer_cast<LocalCollection<std::string, DummyBase>>(basePtr);
    REQUIRE(local != nullptr);

    auto node = std::make_shared<DummyBase>(9);
    REQUIRE(local->add("node", node));
    CHECK(local->contains("node"));
}
