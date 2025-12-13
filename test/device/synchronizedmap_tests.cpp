#include <catch2/catch_test_macros.hpp>

#include <sti/utils/SynchronizedMap.h>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

using STI::Utils::DefaultSynchronizedMapPolicy;
using STI::Utils::SynchronizedMap;
using STI::Utils::SynchronizedMapEvent;
using STI::Utils::SynchronizedMapListener;
using STI::Utils::SynchronizedMapPolicy;

namespace {
struct Dummy {
    int id;
};

// Reject keys that do not start with "ok" and forbid replacements.
class RejectingPolicy : public SynchronizedMapPolicy<std::string> {
public:
    bool include(const std::string& key) const override { return key.rfind("ok", 0) == 0; }
    bool replace(const std::string& /*oldKey*/, const std::string& /*newKey*/) const override { return false; }
};

class RecordingListener : public SynchronizedMapListener<std::string> {
public:
    using EventType = SynchronizedMapEvent<std::string>::Type;

    void add(const std::string& key) override { record(key, EventType::Add); }
    void remove(const std::string& key) override { record(key, EventType::Remove); }
    void refresh() override { record("", EventType::Refresh); }

    bool waitFor(std::size_t expected, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return events.size() >= expected; });
    }

    std::vector<std::pair<std::string, EventType>> events;

private:
    void record(const std::string& key, EventType type) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            events.emplace_back(key, type);
        }
        cv.notify_all();
    }

    std::mutex mutex;
    std::condition_variable cv;
};
} // namespace

TEST_CASE("SynchronizedMap: basic add/get/replace") {
    SynchronizedMap<std::string, std::shared_ptr<Dummy>> map;
    auto one = std::make_shared<Dummy>(Dummy{1});
    auto two = std::make_shared<Dummy>(Dummy{2});

    CHECK(map.add("dev1", one));
    CHECK(map.contains("dev1"));
    CHECK(map.size() == 1);

    std::shared_ptr<Dummy> out;
    REQUIRE(map.get("dev1", out));
    CHECK(out->id == 1);

    // Default policy allows replacement of same key.
    CHECK(map.add("dev1", two));
    REQUIRE(map.get("dev1", out));
    CHECK(out->id == 2);
}

TEST_CASE("SynchronizedMap: custom policy blocks add and replacement") {
    auto policy = std::make_shared<RejectingPolicy>();
    SynchronizedMap<std::string, std::shared_ptr<Dummy>> map(policy);

    auto allowed = std::make_shared<Dummy>(Dummy{1});
    auto blocked = std::make_shared<Dummy>(Dummy{2});

    CHECK(map.add("ok.alpha", allowed));
    CHECK(map.contains("ok.alpha"));

    // Reject keys without prefix.
    CHECK_FALSE(map.add("nope", blocked));
    CHECK(map.size() == 1);

    // Reject replacement: value should remain the original pointer.
    CHECK(map.add("ok.alpha", blocked) == true);  // add returns true because key exists
    std::shared_ptr<Dummy> out;
    REQUIRE(map.get("ok.alpha", out));
    CHECK(out == allowed);
}

TEST_CASE("SynchronizedMap: getKeys/getValues and cleanup") {
    SynchronizedMap<std::string, std::shared_ptr<Dummy>> map;
    map.add("keep", std::make_shared<Dummy>(Dummy{1}));
    map.add("drop", std::make_shared<Dummy>(Dummy{2}));

    std::set<std::string> keys;
    map.getKeys(keys);
    CHECK(keys.size() == 2);
    std::vector<std::shared_ptr<Dummy>> values;
    map.getValues(keys, values);
    CHECK(values.size() == 2);

    // Switch to policy that only keeps the "keep" key; cleanup should remove "drop".
    class KeepOnlyPolicy : public SynchronizedMapPolicy<std::string> {
    public:
        bool include(const std::string& key) const override { return key == "keep"; }
        bool replace(const std::string& oldKey, const std::string& newKey) const override {
            return oldKey == newKey;
        }
    };
    auto policy = std::make_shared<KeepOnlyPolicy>();
    map.setPolicy(policy);
    map.cleanup();
    CHECK(map.contains("keep") == true);
    CHECK(map.contains("drop") == false);
    CHECK(map.size() == 1);
}

TEST_CASE("SynchronizedMap: clear removes all and emits refresh event") {
    SynchronizedMap<std::string, std::shared_ptr<Dummy>> map;
    auto listener = std::make_shared<RecordingListener>();
    map.addListener(listener);

    map.add("a", std::make_shared<Dummy>(Dummy{1}));
    map.add("b", std::make_shared<Dummy>(Dummy{2}));
    map.remove("a");
    map.clear();

    REQUIRE(listener->waitFor(5, std::chrono::milliseconds(500)));
    REQUIRE(listener->events.size() == 5);

    using Type = SynchronizedMapEvent<std::string>::Type;
    std::vector<Type> types;
    for (auto& evt : listener->events) {
        types.push_back(evt.second);
    }
    std::vector<Type> expected{Type::Add, Type::Add, Type::Remove, Type::Remove, Type::Refresh};
    CHECK(types == expected);
}
