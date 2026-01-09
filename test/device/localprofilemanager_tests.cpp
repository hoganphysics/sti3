#include <catch2/catch_test_macros.hpp>

#include "../../src/device/src/LocalProfileManager.h"
#include "../../src/device/src/ProfileTarget.h"
#include <sti/device/Device.h>
#include <sti/utils/LocalCollection.h>
#include <sti/utils/MixedValue.h>

#include "fileholder_tests_support.h"

#include <filesystem>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <vector>

using STI::Device::Device;
using STI::Device::DeviceCollection;
using STI::Device::DeviceID;
using STI::Device::LocalProfileManager;
using STI::Device::Profile;
using STI::Device::ProfileManager;
using STI::Device::ProfileTarget;
using STI::Device::ProfileType;
using STI::Device::PersistenceTarget;
using STI::Utils::LocalCollection;
using STI::Utils::MixedValue;

std::string getProfileFilename(const std::string& profileName);

namespace {

class RecordingProfileTarget : public ProfileTarget {
public:
    explicit RecordingProfileTarget(bool loadReturn = true, bool saveReturn = true)
        : loadReturn(loadReturn), saveReturn(saveReturn) {}

    bool loadProfile(const std::shared_ptr<Profile>& profile) override {
        loadedProfiles.push_back(profile);
        return loadReturn;
    }

    bool saveProfile(const std::shared_ptr<Profile>& profile) override {
        savedProfiles.push_back(profile);
        return saveReturn;
    }

    bool loadReturn;
    bool saveReturn;
    std::vector<std::shared_ptr<Profile>> loadedProfiles;
    std::vector<std::shared_ptr<Profile>> savedProfiles;
};

class PopulatingProfileTarget : public ProfileTarget {
public:
    std::function<void(const std::shared_ptr<Profile>&)> onSave;
    bool loadReturn{true};
    bool saveReturn{true};
    std::vector<std::shared_ptr<Profile>> savedProfiles;

    bool loadProfile(const std::shared_ptr<Profile>& profile) override {
        loadedProfiles.push_back(profile);
        return loadReturn;
    }

    bool saveProfile(const std::shared_ptr<Profile>& profile) override {
        savedProfiles.push_back(profile);
        if (onSave) {
            onSave(profile);
        }
        return saveReturn;
    }

    std::vector<std::shared_ptr<Profile>> loadedProfiles;
};

class StubProfileManager : public ProfileManager {
public:
    bool loadReturn{true};
    bool saveReturn{true};
    std::vector<std::tuple<std::string, ProfileType, bool>> loadCalls;
    std::vector<std::tuple<std::string, ProfileType, bool>> saveCalls;

    void getProfiles(std::set<std::string>&) const override {}

    bool getProfile(const std::string&, std::shared_ptr<Profile>&) const override { return false; }

    bool saveProfile(const std::shared_ptr<Profile>&) override { return false; }

    bool setReadOnly(const std::string&, bool) override { return false; }

    bool loadProfile(const std::string& name, const ProfileType& type, bool loadDependentDevices) override {
        loadCalls.emplace_back(name, type, loadDependentDevices);
        return loadReturn;
    }

    bool saveCurrentProfile(const std::string& name, const ProfileType& type, bool saveDependentDevices) override {
        saveCalls.emplace_back(name, type, saveDependentDevices);
        return saveReturn;
    }
};

class StubDevice : public Device {
public:
    StubDevice(const DeviceID& id, const std::shared_ptr<ProfileManager>& profileManager)
        : id(id), profileManager(profileManager) {}

    const DeviceID getID() const override { return id; }
    void kill() override {}
    void getMessageDispatcher(std::shared_ptr<STI::Device::DeviceMessageDispatcher>&) override {}
    bool getEngineScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>&) override { return false; }
    void getChannelManager(std::shared_ptr<STI::Device::ChannelManager>&) override {}
    void getAttributeManager(std::shared_ptr<STI::Device::AttributeManager>&) override {}
    bool getPersistenceManager(std::shared_ptr<STI::Device::PersistenceManager>&) override { return false; }

    bool getProfileManager(std::shared_ptr<ProfileManager>& manager) override {
        manager = profileManager;
        return (manager != nullptr);
    }

    bool getTaskManager(std::shared_ptr<STI::Device::TaskManager>&) override { return false; }
    bool getLogManager(std::shared_ptr<STI::Device::LogManager>&) override { return false; }
    void attachMessageListenerForwarder(const std::shared_ptr<STI::Device::DeviceMessageListenerForwarder>&) override {}

    bool write(short, const MixedValue&) override { return false; }
    bool read(short, MixedValue&) override { return false; }
    bool read(short, const MixedValue&, MixedValue&) override { return false; }
    void stopRW() override {}
    std::string getAttribute(const std::string&) override { return {}; }
    bool setAttribute(const std::string&, const std::string&) override { return false; }
    bool getAttribute(const std::string&, std::shared_ptr<STI::Device::Attribute>&) override { return false; }
    bool refresh() override { return true; }
    void activate() override {}
    void disable() override {}
    void getCollection(std::shared_ptr<STI::Utils::Collection<DeviceID, Device>>& collection) override { collection.reset(); }

private:
    DeviceID id;
    std::shared_ptr<ProfileManager> profileManager;
};

DeviceID makeDeviceID(const std::string& name = "Device", const std::string& address = "127.0.0.1",
                      unsigned short module = 1, const std::string& targetServer = "server") {
    return DeviceID(name, address, module, targetServer);
}

} // namespace

TEST_CASE("LocalProfileManager saves profiles and enumerates names", "[profile][localprofilemanager]") {
    auto manager = std::make_shared<LocalProfileManager>(makeDeviceID(), nullptr);
    auto* persistence = static_cast<PersistenceTarget*>(manager.get());

    std::size_t callbackCount = 0;
    persistence->setPersistenceCallback([&callbackCount]() { ++callbackCount; });

    auto profile = std::make_shared<Profile>();
    profile->name = "profile1";
    profile->type = ProfileType::All;

    REQUIRE(manager->saveProfile(profile));
    CHECK(callbackCount == 1);

    std::set<std::string> names;
    manager->getProfiles(names);
    CHECK(names.count("profile1") == 1);

    std::shared_ptr<Profile> fetched;
    REQUIRE(manager->getProfile("profile1", fetched));
    CHECK(fetched == profile);

    auto replacement = std::make_shared<Profile>(*profile);
    replacement->name = "profile1";
    replacement->attributeData["new"] = "value";
    REQUIRE(manager->saveProfile(replacement));
    CHECK(callbackCount == 2);
    REQUIRE(manager->getProfile("profile1", fetched));
    CHECK(fetched == replacement);
    CHECK(fetched->attributeData.at("new") == "value");
}

TEST_CASE("LocalProfileManager loadProfile clones types and propagates target failures", "[profile][localprofilemanager]") {
    auto manager = std::make_shared<LocalProfileManager>(makeDeviceID(), nullptr);
    auto* persistence = static_cast<PersistenceTarget*>(manager.get());
    persistence->setPersistenceCallback([]() {});

    auto cached = std::make_shared<Profile>();
    cached->name = "base";
    cached->type = ProfileType::All;
    cached->attributeData["key"] = "value";
    cached->channelData[1] = MixedValue(3.0);
    REQUIRE(manager->saveProfile(cached));

    auto successTarget = std::make_shared<RecordingProfileTarget>(true, true);
    auto failingTarget = std::make_shared<RecordingProfileTarget>(false, true);
    manager->addProfileTarget(successTarget);
    manager->addProfileTarget(failingTarget);

    std::shared_ptr<Profile> fetched;
    REQUIRE(manager->getProfile("base", fetched));

    CHECK_FALSE(manager->loadProfile("base", ProfileType::Channel, false));

    REQUIRE(successTarget->loadedProfiles.size() == 1);
    auto loaded = successTarget->loadedProfiles.front();
    CHECK(loaded != fetched);
    CHECK(loaded->type == ProfileType::Channel);
    CHECK(loaded->attributeData == cached->attributeData);
    CHECK(loaded->channelData == cached->channelData);
    CHECK(failingTarget->loadedProfiles.size() == 1);
}

TEST_CASE("LocalProfileManager saveCurrentProfile aggregates data from targets", "[profile][localprofilemanager]") {
    auto manager = std::make_shared<LocalProfileManager>(makeDeviceID(), nullptr);
    auto* persistence = static_cast<PersistenceTarget*>(manager.get());

    std::size_t callbackCount = 0;
    persistence->setPersistenceCallback([&callbackCount]() { ++callbackCount; });

    auto targetA = std::make_shared<PopulatingProfileTarget>();
    targetA->onSave = [](const std::shared_ptr<Profile>& profile) { profile->attributeData["alpha"] = "1"; };
    auto targetB = std::make_shared<PopulatingProfileTarget>();
    targetB->onSave = [](const std::shared_ptr<Profile>& profile) { profile->channelData[5] = MixedValue(2.0); };

    manager->addProfileTarget(targetA);
    manager->addProfileTarget(targetB);

    REQUIRE(manager->saveCurrentProfile("current", ProfileType::All, false));
    CHECK(callbackCount == 1);
    std::shared_ptr<Profile> stored;
    REQUIRE(manager->getProfile("current", stored));
    CHECK(stored->attributeData["alpha"] == "1");
    CHECK(stored->channelData[5] == MixedValue(2.0));

    targetB->saveReturn = false;
    CHECK_FALSE(manager->saveCurrentProfile("next", ProfileType::All, false));
    CHECK(callbackCount == 2);
    std::shared_ptr<Profile> next;
    REQUIRE(manager->getProfile("next", next));
    CHECK(next->channelData[5] == MixedValue(2.0));
}

TEST_CASE("LocalProfileManager loads dependent device profiles for matching targets", "[profile][localprofilemanager]") {
    auto collection = std::make_shared<LocalCollection<DeviceID, Device>>();
    std::shared_ptr<DeviceCollection> baseCollection = collection;

    auto serverID = makeDeviceID("Server", "127.0.0.1", 10, "central");
    LocalProfileManager manager(serverID, baseCollection);
    auto* persistence = static_cast<PersistenceTarget*>(&manager);
    persistence->setPersistenceCallback([]() {});

    auto profile = std::make_shared<Profile>();
    profile->name = "shared";
    profile->type = ProfileType::All;
    REQUIRE(manager.saveProfile(profile));

    auto matchingMgr = std::make_shared<StubProfileManager>();
    auto matchingDevice = std::make_shared<StubDevice>(makeDeviceID("Child", "127.0.0.1", 1, serverID.getID()), matchingMgr);
    REQUIRE(collection->add(matchingDevice->getID(), matchingDevice));

    auto otherMgr = std::make_shared<StubProfileManager>();
    auto otherDevice = std::make_shared<StubDevice>(makeDeviceID("Other", "127.0.0.1", 2, "different-server"), otherMgr);
    REQUIRE(collection->add(otherDevice->getID(), otherDevice));

    REQUIRE(manager.loadProfile("shared", ProfileType::All, true));

    REQUIRE(matchingMgr->loadCalls.size() == 1);
    auto [name, type, dependent] = matchingMgr->loadCalls.front();
    CHECK(name == ("@" + serverID.getID() + "#shared"));
    CHECK(type == ProfileType::All);
    CHECK(dependent);
    CHECK(otherMgr->loadCalls.empty());
}

TEST_CASE("LocalProfileManager persists profiles to disk and reloads them", "[profile][localprofilemanager]") {
    auto tempDir = fileholder_test_support::TempDir("localprofilemanager-");
    auto serverID = makeDeviceID();

    auto saver = std::make_shared<LocalProfileManager>(serverID, nullptr);
    auto* persistence = static_cast<PersistenceTarget*>(saver.get());
    persistence->setPersistenceCallback([]() {});

    auto profile = std::make_shared<Profile>();
    profile->name = "unsafe:name with spaces";
    profile->type = ProfileType::Attribute;
    profile->attributeData["k"] = "v";
    REQUIRE(saver->saveProfile(profile));
    REQUIRE(persistence->save(tempDir.path.string()));

    auto expectedFile = tempDir.path / getProfileFilename(profile->name);
    CHECK(std::filesystem::exists(expectedFile));

    auto loader = std::make_shared<LocalProfileManager>(serverID, nullptr);
    auto* loaderPersistence = static_cast<PersistenceTarget*>(loader.get());
    loaderPersistence->load(tempDir.path.string());

    std::shared_ptr<Profile> loaded;
    REQUIRE(loader->getProfile("unsafe:name with spaces", loaded));
    CHECK(loaded->attributeData["k"] == "v");
}
