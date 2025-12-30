#include <catch2/catch_test_macros.hpp>

#include <sti/utils/Distributer.h>
#include <sti/utils/BinaryData.h>
#include <sti/utils/LocalCollection.h>
#include <sti/utils/MixedValue.h>
#include <sti/device/DeviceCollection.h>
#include <sti/LocalDevice.h>

#include "fileholder_tests_support.h"

#include <fstream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

using STI::Device::Device;
using STI::Device::DeviceCollection;
using STI::Device::DeviceID;
using STI::Device::LocalDevice;
using STI::Utils::BinaryData;
using STI::Utils::Distributer;
using STI::Utils::LocalCollection;
using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;

namespace {

class SimpleLocalDevice : public LocalDevice {
public:
    SimpleLocalDevice(const std::string& name, unsigned short module)
        : LocalDevice(name, "127.0.0.1", module, "target") {}

    bool writeChannel(short channel, const MixedValue& value) override {
        lastWrite[channel] = value;
        auto it = writeResults.find(channel);
        return it != writeResults.end() ? it->second : true;
    }

    bool readChannel(short channel, const MixedValue& value, MixedValue& data) override {
        lastRead[channel] = value;
        auto it = readResponses.find(channel);
        if (it != readResponses.end()) {
            data = it->second;
            return true;
        }
        if (defaultRead.has_value()) {
            data = *defaultRead;
            return true;
        }
        return false;
    }

    void setWriteResult(short channel, bool success) { writeResults[channel] = success; }
    void setReadResponse(short channel, const MixedValue& response) { readResponses[channel] = response; }
    void setDefaultReadResponse(const MixedValue& response) { defaultRead = response; }

    std::map<short, MixedValue> lastWrite;
    std::map<short, MixedValue> lastRead;

private:
    std::map<short, bool> writeResults;
    std::map<short, MixedValue> readResponses;
    std::optional<MixedValue> defaultRead;
};

using LocalDeviceCollection = LocalCollection<DeviceID, Device>;

std::shared_ptr<LocalDeviceCollection> getCollection(const std::shared_ptr<SimpleLocalDevice>& device) {
    std::shared_ptr<DeviceCollection> base;
    device->getCollection(base);
    return std::dynamic_pointer_cast<LocalDeviceCollection>(base);
}

void connectPartners(const std::shared_ptr<SimpleLocalDevice>& a, const std::shared_ptr<SimpleLocalDevice>& b) {
    a->addPartner(b->getID());
    b->addPartner(a->getID());
}

void connectPartners(const std::vector<std::shared_ptr<SimpleLocalDevice>>& devices) {
    for (std::size_t i = 0; i < devices.size(); ++i) {
        for (std::size_t j = i + 1; j < devices.size(); ++j) {
            connectPartners(devices[i], devices[j]);
        }
    }
}

} // namespace

TEST_CASE("Distributer distributes devices without self references") {
    auto devA = std::make_shared<SimpleLocalDevice>("DeviceA", 1);
    auto devB = std::make_shared<SimpleLocalDevice>("DeviceB", 2);

    connectPartners(devA, devB);

    Distributer<DeviceID, Device> distributer;
    REQUIRE(distributer.add(devA->getID(), devA));
    REQUIRE(distributer.add(devB->getID(), devB));
    CHECK(distributer.numberOfNodes() == 2);

    auto collectionA = getCollection(devA);
    auto collectionB = getCollection(devB);
    REQUIRE(collectionA);
    REQUIRE(collectionB);

    std::shared_ptr<Device> other;
    REQUIRE(collectionA->get(devB->getID(), other));
    CHECK(other == devB);
    CHECK_FALSE(collectionA->contains(devA->getID()));

    REQUIRE(collectionB->get(devA->getID(), other));
    CHECK(other == devA);
    CHECK_FALSE(collectionB->contains(devB->getID()));
}

TEST_CASE("Distributer redistributes existing nodes after collector clears state") {
    auto devA = std::make_shared<SimpleLocalDevice>("DeviceA", 1);
    auto devB = std::make_shared<SimpleLocalDevice>("DeviceB", 2);

    connectPartners(devA, devB);

    Distributer<DeviceID, Device> distributer;
    REQUIRE(distributer.add(devA->getID(), devA));
    REQUIRE(distributer.add(devB->getID(), devB));

    auto collectionA = getCollection(devA);
    REQUIRE(collectionA);
    collectionA->clear();
    CHECK(collectionA->size() == 0);

    distributer.distribute();

    std::shared_ptr<Device> other;
    REQUIRE(collectionA->get(devB->getID(), other));
    CHECK(other == devB);
}

TEST_CASE("Distributer remove erases nodes from all collectors") {
    auto devA = std::make_shared<SimpleLocalDevice>("DeviceA", 1);
    auto devB = std::make_shared<SimpleLocalDevice>("DeviceB", 2);
    auto devC = std::make_shared<SimpleLocalDevice>("DeviceC", 3);

    connectPartners({devA, devB, devC});

    Distributer<DeviceID, Device> distributer;
    REQUIRE(distributer.add(devA->getID(), devA));
    REQUIRE(distributer.add(devB->getID(), devB));
    REQUIRE(distributer.add(devC->getID(), devC));

    auto collectionC = getCollection(devC);
    REQUIRE(collectionC);
    CHECK(collectionC->contains(devA->getID()));
    CHECK(collectionC->contains(devB->getID()));

    REQUIRE(distributer.remove(devB->getID()));
    CHECK(distributer.numberOfNodes() == 2);
    CHECK_FALSE(collectionC->contains(devB->getID()));

    auto collectionA = getCollection(devA);
    REQUIRE(collectionA);
    CHECK_FALSE(collectionA->contains(devB->getID()));
}

TEST_CASE("Distributer clearAll empties collector caches without dropping owned nodes") {
    auto devA = std::make_shared<SimpleLocalDevice>("DeviceA", 1);
    auto devB = std::make_shared<SimpleLocalDevice>("DeviceB", 2);
    auto devC = std::make_shared<SimpleLocalDevice>("DeviceC", 3);

    connectPartners({devA, devB, devC});

    Distributer<DeviceID, Device> distributer;
    REQUIRE(distributer.add(devA->getID(), devA));
    REQUIRE(distributer.add(devB->getID(), devB));
    REQUIRE(distributer.add(devC->getID(), devC));

    auto collectionA = getCollection(devA);
    auto collectionB = getCollection(devB);
    auto collectionC = getCollection(devC);
    REQUIRE(collectionA);
    REQUIRE(collectionB);
    REQUIRE(collectionC);

    CHECK(collectionA->size() == 2);
    CHECK(collectionB->size() == 2);
    CHECK(collectionC->size() == 2);

    distributer.clearAll();

    CHECK(collectionA->size() == 0);
    CHECK(collectionB->size() == 0);
    CHECK(collectionC->size() == 0);
    CHECK(distributer.numberOfNodes() == 3);
}

TEST_CASE("Distributer supports BinaryData and File MixedValue writes through distributed devices") {
    auto source = std::make_shared<SimpleLocalDevice>("Source", 1);
    auto target = std::make_shared<SimpleLocalDevice>("Target", 2);

    connectPartners(source, target);

    target->addOutputChannel(7, MixedValueType::Binary, "binary-out");
    target->addOutputChannel(8, MixedValueType::File, "file-out");

    Distributer<DeviceID, Device> distributer;
    REQUIRE(distributer.add(source->getID(), source));
    REQUIRE(distributer.add(target->getID(), target));

    auto collection = getCollection(source);
    REQUIRE(collection);
    std::shared_ptr<Device> remoteBase;
    REQUIRE(collection->get(target->getID(), remoteBase));
    auto remoteTarget = std::dynamic_pointer_cast<SimpleLocalDevice>(remoteBase);
    REQUIRE(remoteTarget);

    auto binary = std::make_shared<BinaryData>();
    auto* buffer = binary->allocate<char>(4);
    buffer[0] = 's';
    buffer[1] = 't';
    buffer[2] = 'i';
    buffer[3] = '3';

    REQUIRE(remoteTarget->write(7, MixedValue(binary)));
    auto storedBinary = remoteTarget->lastWrite.at(7);
    CHECK(storedBinary.isType(MixedValueType::Binary));
    auto writtenBinary = storedBinary.getBinary();
    REQUIRE(writtenBinary);
    char* readBuffer = nullptr;
    REQUIRE(writtenBinary->getBytes(readBuffer));
    CHECK(readBuffer[0] == 's');
    CHECK(writtenBinary->length() == 4);

    fileholder_test_support::TempDir tempDir;
    auto filePath = tempDir.path / "payload.txt";
    {
        std::ofstream out(filePath);
        out << "file-data";
    }
    auto fileID = fileholder_test_support::makeFileID(target->getID(), tempDir.path, "payload.txt");

    REQUIRE(remoteTarget->write(8, MixedValue(fileID)));
    auto storedFile = remoteTarget->lastWrite.at(8);
    CHECK(storedFile.isType(MixedValueType::File));
    CHECK(storedFile.getFileID().filename == "payload.txt");
}
